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
