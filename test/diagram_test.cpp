#include <gtest/gtest.h>
#include <diagram.h>
#include <amplitude.h>

using absi::Interval;
using namespace diagram;

class DiagramTest : public testing::Test
{
public:
    Diagram *leaf = new Diagram(0);
    Diagram *eig0 = new Diagram(1);
    Diagram *eig1 = new Diagram(1);
    Diagram *dgm = new Diagram(2);
};

TEST_F(DiagramTest, testStateVector)
{
    for (auto i = 0; i < 100; i++)
    {
        ampl::ConcreteState state(2);
        ampl::randomize_state(state);
        auto diagram = Diagram::from_state_vector(state);
        auto evaluated_state = diagram->evaluate();
        for (auto i = 0; i < state.size(); i++)
        {
            EXPECT_EQ(evaluated_state[i], state[i])
                << "Failed at index " << i << " against " << evaluated_state[i].to_string();
        }
    }
}

TEST_F(DiagramTest, testEvaluate)
{
    eig0->lefto(leaf);
    dgm->lefto(eig0);
    dgm->righto(eig0, 2.);
    auto a = leaf->evaluate()[0], b = eig0->evaluate()[0], c = eig0->evaluate()[1];
    EXPECT_EQ(absi::one, a);
    EXPECT_EQ(absi::one, b) << b.to_string();
    EXPECT_EQ(absi::zero, c);
}

TEST_F(DiagramTest, doubleEvaluation)
{
    auto d = Diagram::random(3);
    auto v1 = d->evaluate(), v2 = d->evaluate();
    for (auto i = 0; i < v1.size(); i++)
    {
        EXPECT_EQ(v1[i], v2[i]);
    }
    delete d;
}

TEST_F(DiagramTest, eig0)
{
    auto d = Diagram::eig0(3);
    auto vec = d->evaluate();
    for (auto i = 0; i < vec.size(); i++)
    {
        EXPECT_EQ(i == 0 ? absi::one : absi::zero, vec[i]);
    }
}

TEST_F(DiagramTest, build)
{
    auto two = Interval(2.);
    eig0->lefto(leaf);
    dgm->lefto(eig0);
    dgm->righto(eig0, two);
    auto vec = dgm->evaluate(); // (1, 0, 2, 0)
    EXPECT_EQ(absi::one, vec[0]);
    EXPECT_EQ(absi::zero, vec[1]) << vec[1].to_string();
    EXPECT_EQ(two, vec[2]) << vec[2].to_string();
    EXPECT_EQ(absi::zero, vec[3]) << vec[3].to_string();
}

TEST_F(DiagramTest, testClone)
{
    eig0->lefto(leaf);
    dgm->lefto(eig0);
    dgm->righto(eig0);
    auto clone = dgm->clone();
    auto dgm_eval = dgm->evaluate();
    auto clone_eval = clone->evaluate();
    for (auto i = 0; i < dgm_eval.size(); i++)
    {
        EXPECT_EQ(dgm_eval[i], clone_eval[i]) << "Evaluations not initially equal at index " << i;
    }
    clone->left[0].x = Interval(2.);
    dgm_eval = dgm->evaluate();
    clone_eval = clone->evaluate();
    EXPECT_NE(dgm_eval[0], clone_eval[0]);
    for (auto i = 1; i < dgm_eval.size(); i++)
    {
        EXPECT_EQ(dgm_eval[i], clone_eval[i]) << "Evaluations not equal after change at index " << i;
    }
}

TEST_F(DiagramTest, additiveness)
{
    auto two = Interval(2.);
    auto minus_three = Interval(-3.);
    auto minus_one = Interval(2. + (-3.));
    eig0->lefto(leaf, two);
    eig0->lefto(leaf, minus_three);
    auto vec = eig0->evaluate();
    EXPECT_EQ(minus_one, vec[0]);
    EXPECT_EQ(absi::zero, vec[1]);
}

TEST_F(DiagramTest, get_node_pointers_at_height)
{
    eig0->lefto(leaf);
    eig1->righto(leaf);
    dgm->lefto(eig0);
    dgm->righto(eig1);
    auto vec = dgm->get_node_pointers_at_height(1);
    EXPECT_EQ(2, vec.size());
    if (vec[0] != eig0)
    {
        std::swap(vec[0], vec[1]);
    }
    EXPECT_EQ(eig0, vec[0]);
    EXPECT_EQ(eig1, vec[1]);
}

TEST_F(DiagramTest, random_is_norm_lower_than_one)
{
    auto d = Diagram::random(2);
    auto vec = d->evaluate();
    for (auto i : vec)
    {
        EXPECT_LE(i.norm(), 1.);
    }
    delete d;
}

TEST_F(DiagramTest, random_assert_variability)
{
    const auto n = 100;
    size_t counts[n][5];
    bool is_not_duplicate[n];
    for (auto i = 0; i < n; i++)
    {
        auto d = Diagram::random(5);
        for (auto j = 0; j < 5; j++)
        {
            counts[i][j] = d->count_nodes_at_height(j);
        }
        delete d;
    }
    for (auto i = 0; i < n; i++)
    {
        bool broke = false;
        for (auto k = 0; k < i; k++)
        {
            for (auto j = 0; j < 5; j++)
            {
                if (counts[i][j] != counts[i][k])
                {
                    broke = true;
                    break;
                }
            }
        }
    }
    for (auto i = 0; i < n; i++)
    {
        EXPECT_TRUE(true);
    }
}

TEST_F(DiagramTest, random_assert_boundaries)
{
    const auto n = 100;
    for (auto j = 0; j < n; j++)
    {
        auto d = Diagram::random(5);
        for (auto i = 0; i < 5; i++)
        {
            EXPECT_GE(d->count_nodes_at_height(i), 0);
            EXPECT_LE(d->count_nodes_at_height(i), pow(CHILDREN_NUMBER_AMBITION, i + 3));
        }
        delete d;
    }
}

TEST_F(DiagramTest, enclosure)
{
    const auto n = 3;
    for (auto i = 0; i < 1000; i++)
    {
        auto d = Diagram::random(n);
        auto v = d->evaluate();

        auto real_rho = v[0];
        for (auto i = 1; i < pwrtwo(n); i++)
        {
            real_rho = real_rho | v[i];
        }

        auto rho = d->enclosure();
        EXPECT_EQ(rho, real_rho);
        EXPECT_EQ(rho, d->enclosure());
        delete d;
    }
}

/* TEST_F(DiagramTest, testEnclosureUpdate)
{
    const auto n = 2;
    auto c = Diagram::randomPointer(n-1);
    auto d = Diagram::randomPointer(n);
    auto rho = d->enclosure();

    // This necessarily adds uncertainty.
    d->lefto(c);
    d->righto(c);

    auto v = d->evaluate();
    auto real_rho = v[0];
    for (auto i = 1; i < pwrtwo(n); i++) {
        real_rho = real_rho | v[i];
    }
    auto new_rho = d->enclosure();
    // EXPECT_FALSE(true);
    EXPECT_EQ(new_rho, real_rho) << new_rho.to_string() << " != " << real_rho.to_string();
} */