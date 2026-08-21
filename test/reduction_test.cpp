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
