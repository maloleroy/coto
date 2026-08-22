#include <gtest/gtest.h>
#include <reduction.h>

TEST(ReductionTest, cut_dead_branches)
{
    ampl::Amplitude v[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    auto d = diagram::Diagram::from_state_vector(ampl::ConcreteState(3, v));
    EXPECT_EQ(d->count_nodes_at_height(0), 0);
    EXPECT_EQ(d->count_nodes_at_height(1), 4);
    EXPECT_EQ(d->count_nodes_at_height(2), 2);
    EXPECT_EQ(d->count_nodes_at_height(3), 1);
    reduction::cut_dead_branches(d);
    EXPECT_EQ(d->count_nodes_at_height(0), 0);
    EXPECT_EQ(d->count_nodes_at_height(1), 0);
    EXPECT_EQ(d->count_nodes_at_height(2), 0);
    EXPECT_EQ(d->count_nodes_at_height(3), 1);

    auto ev = d->evaluate();
    for (auto i = 0; i < ev.size(); i++)
    {
        EXPECT_EQ(ev[i], 0) << "At index " << i << ", got " << ev[i].to_string() << ", expected 0";
    }
    delete d;
}

TEST(ReductionTest, cut_dead_branches_reduces_size)
{
    ampl::Amplitude v[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    auto d = diagram::Diagram::from_state_vector(ampl::ConcreteState(3, v));
    size_t before = d->memory_usage();
    reduction::cut_dead_branches(d);
    size_t after = d->memory_usage();
    EXPECT_LT(after, before);
    delete d;
}

TEST(ReductionTest, cut_dead_branches_preserves_shared_live_children)
{
    auto *live = diagram::Diagram::eig0(1);
    auto *dead = new diagram::Diagram(1);
    auto *d = new diagram::Diagram(2);
    d->lefto(dead);
    d->lefto(live);
    d->righto(dead);
    d->righto(live);

    reduction::cut_dead_branches(d);

    ASSERT_EQ(d->left.size(), 1);
    ASSERT_EQ(d->right.size(), 1);
    EXPECT_EQ(d->left.front().d, live);
    EXPECT_EQ(d->right.front().d, live);
    delete d;
}

TEST(ReductionTest, force_merge_contains_inputs_with_disjoint_children)
{
    auto *a = new diagram::Diagram(1);
    auto *b = new diagram::Diagram(1);
    a->lefto(diagram::Diagram::eig0(0));
    b->righto(diagram::Diagram::eig0(0));

    auto *merged = reduction::force_merge(*a, *b);
    const auto evaluation = merged->evaluate();
    EXPECT_TRUE(evaluation[0].contains(0));
    EXPECT_TRUE(evaluation[0].contains(1));
    EXPECT_TRUE(evaluation[1].contains(0));
    EXPECT_TRUE(evaluation[1].contains(1));

    delete merged;
    delete a;
    delete b;
}

TEST(ReductionTest, force_merge_sums_duplicate_branches_before_widening)
{
    auto *a = new diagram::Diagram(1);
    auto *b = new diagram::Diagram(1);
    auto *terminal = diagram::Diagram::eig0(0);
    a->lefto(terminal, 1);
    a->lefto(terminal, 2);
    b->righto(terminal, 4);

    auto *merged = reduction::force_merge(*a, *b);
    const auto evaluation = merged->evaluate();
    EXPECT_TRUE(evaluation[0].contains(0));
    EXPECT_TRUE(evaluation[0].contains(3));
    EXPECT_TRUE(evaluation[1].contains(0));
    EXPECT_TRUE(evaluation[1].contains(4));

    delete merged;
    delete a;
    delete b;
}

TEST(ReductionTest, force_merge_preserves_uncertainty_from_duplicate_abstract_branches)
{
    auto *child = new diagram::Diagram(1);
    child->lefto(diagram::Diagram::eig0(0), absi::Interval(-1, 1));
    auto *a = new diagram::Diagram(2);
    a->lefto(child, 1);
    a->lefto(child, -1);
    auto *b = diagram::Diagram::eig0(2);
    const auto before = a->evaluate();

    auto *merged = reduction::force_merge(*a, *b);
    const auto after = merged->evaluate();
    for (size_t index = 0; index < before.size(); ++index)
        EXPECT_TRUE(after[index].contains_interval(before[index])) << "index=" << index;

    delete merged;
    delete a;
    delete b;
}

TEST(ReductionTest, unitary_image_enclosure_contains_hadamard_outputs)
{
    const absi::Interval uncertain(
        cartesian::RealInterval{-1, 2}, cartesian::RealInterval{-0.5, 0.25});
    auto *diagram = new diagram::Diagram(2);
    auto *child = diagram::Diagram::eig0(1);
    diagram->lefto(child, uncertain);

    reduction::enclose_unitary_image(diagram, 1);

    const auto evaluation = diagram->evaluate();
    for (const auto &amplitude : evaluation)
    {
        EXPECT_TRUE(amplitude.contains(ampl::Amplitude(2 * std::sqrt(2.0), 0)));
        EXPECT_TRUE(amplitude.contains(ampl::Amplitude(-2 * std::sqrt(2.0), 0)));
    }
    delete diagram;
}

TEST(ReductionTest, node_budget_contains_original_state_and_reduces_memory)
{
    const ampl::Amplitude values[] = {1, -2, 3, -4, 5, -6, 7, -8};
    const ampl::ConcreteState state(3, values);
    auto *diagram = diagram::Diagram::from_state_vector(state);
    const size_t memory_before = diagram->memory_usage();

    const size_t merges = reduction::max_nodes_per_level(diagram, 1);

    EXPECT_GT(merges, 0);
    for (size_t height = 1; height < diagram->height; ++height)
        EXPECT_LE(diagram->count_nodes_at_height(height), 1);
    EXPECT_LT(diagram->memory_usage(), memory_before);
    const auto approximate = diagram->evaluate();
    for (size_t index = 0; index < state.size(); ++index)
        EXPECT_TRUE(approximate[index].contains(state[index]))
            << "Reduced state lost amplitude " << state[index] << " at index " << index;
    delete diagram;
}

TEST(ReductionTest, replacement_is_scoped_to_the_reduced_root)
{
    auto *first = new diagram::Diagram(1);
    auto *second = new diagram::Diagram(1);
    first->lefto(diagram::Diagram::eig0(0));
    second->righto(diagram::Diagram::eig0(0));
    auto *reduced_root = new diagram::Diagram(2);
    reduced_root->lefto(first);
    reduced_root->righto(second);
    auto *other_root = new diagram::Diagram(2);
    other_root->lefto(first, 2);
    const auto before = other_root->evaluate();

    reduction::max_nodes_per_level(reduced_root, 1);

    const auto after = other_root->evaluate();
    for (size_t index = 0; index < before.size(); ++index)
        EXPECT_EQ(after[index], before[index]);
    delete reduced_root;
    delete other_root;
}

TEST(ReductionTest, deterministic_property_check_preserves_concrete_states)
{
    for (size_t height = 2; height <= 5; ++height)
    {
        std::vector<ampl::Amplitude> values(pwrtwo(height));
        for (size_t trial = 0; trial < 8; ++trial)
        {
            for (size_t index = 0; index < values.size(); ++index)
            {
                const double real = static_cast<double>((index * 17 + trial * 7) % 19) - 9.0;
                const double imaginary = static_cast<double>((index * 11 + trial * 5) % 13) - 6.0;
                values[index] = ampl::Amplitude(real / 10.0, imaginary / 10.0);
            }
            const ampl::ConcreteState state(height, values.data());
            auto *diagram = diagram::Diagram::from_state_vector(state);

            reduction::max_nodes_per_level(diagram, 1 + trial % 2);

            const auto approximate = diagram->evaluate();
            for (size_t index = 0; index < state.size(); ++index)
                EXPECT_TRUE(approximate[index].contains(state[index]))
                    << "height=" << height << ", trial=" << trial << ", index=" << index;
            delete diagram;
        }
    }
}
