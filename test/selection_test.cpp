#include <gtest/gtest.h>
#include <selection.h>

using diagram::Diagram;

class SelectionTest : public testing::Test
{
public:
    Diagram *leaf = new Diagram(0);
    Diagram *eig0 = new Diagram(1);
    Diagram *eig1 = new Diagram(1);
    Diagram *dgm = new Diagram(2);
};

TEST_F(SelectionTest, Random)
{
    eig0->lefto(leaf);
    eig1->righto(leaf);
    dgm->lefto(eig0);
    dgm->righto(eig1, 2.);
    EXPECT_EQ(dgm->count_nodes_at_height(1), 2);

    struct selection::Mergees expectedMergees{eig0, eig1};
    struct selection::Mergees m = selection::get_mergees_at_height(1, dgm, selection::MergeesChoiceStrategy::RANDOM);

    EXPECT_EQ(m.a->height, eig0->height);
    EXPECT_EQ(m.b->height, eig0->height);
    EXPECT_EQ(expectedMergees, m);
}

TEST(SelectionStrategyTest, minimum_imprecision_selects_closest_pair)
{
    auto *terminal = Diagram::eig0(0);
    auto *near_a = new Diagram(1);
    auto *near_b = new Diagram(1);
    auto *far = new Diagram(1);
    near_a->lefto(terminal, 1.0);
    near_b->lefto(terminal, 1.1);
    far->lefto(terminal, 10.0);
    auto *root = new Diagram(2);
    root->lefto(near_a);
    root->lefto(near_b);
    root->righto(far);

    const auto selected = selection::get_mergees_at_height(1, root, selection::MIN_IMPRECISION);

    EXPECT_EQ(selected, (selection::Mergees{near_a, near_b}));
    delete root;
}
