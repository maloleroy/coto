#include <gtest/gtest.h>
#include <absi.h>

using polar::PositiveInterval, polar::AngleInterval;

class CartesianTest : public testing::Test
{
public:
    cartesian::Interval a = cartesian::Interval(ampl::one);
    cartesian::Interval b = cartesian::Interval(ampl::zero, ampl::one + ampl::i);
};

TEST_F(CartesianTest, norm)
{
    EXPECT_EQ(a.norm(), ampl::zero_real);
    EXPECT_EQ(b.norm(), ampl::sqrt2);
}

TEST_F(CartesianTest, contains)
{
    EXPECT_TRUE(a.contains(ampl::one));
    EXPECT_FALSE(a.contains(ampl::i));
    EXPECT_TRUE(b.contains(ampl::i));
    EXPECT_FALSE(b.contains(ampl::i * 3.));
}

TEST_F(CartesianTest, equal)
{
    cartesian::Interval c = cartesian::Interval(ampl::zero, ampl::one + ampl::i);
    EXPECT_EQ(b, c);
    EXPECT_EQ(ampl::zero, ampl::zero * ampl::one);
    EXPECT_EQ(ampl::zero, ampl::one * ampl::zero);
}

TEST_F(CartesianTest, join)
{
    auto j = cartesian::zero
    | cartesian::Interval(ampl::i + ampl::one);
    EXPECT_EQ(j, b);
}

class PolarTest : public testing::Test
{
public:
    polar::PositiveInterval pa{5.}, pb{3., 4.}, pa_plus_pb{8., 9.}, pa_times_pb{15., 20.}, pa_join_pb{3., 5.};
    polar::AngleInterval ra{-.25, 1.25}, ra_other{1.75, 1.25}, rb{1}, ra_plus_rb{.75, 1.25}, ra_join_rb{ra};
    polar::Interval i1{pa, ra}, i2{pb, rb}, i1_times_i2{pa_times_pb, ra_plus_rb}, i1_join_i2{pa_join_pb, ra_join_rb};
};

TEST_F(PolarTest, positive_interval_operations)
{
    EXPECT_EQ(pa + pb, pa_plus_pb);
    EXPECT_EQ(pa * pb, pa_times_pb);
    EXPECT_EQ(pa | pb, pa_join_pb);
}

TEST_F(PolarTest, angle_interval_operations)
{
    EXPECT_EQ(ra, ra_other);
    EXPECT_EQ(ra + rb, ra_plus_rb);
    // EXPECT_EQ(ra * rb, ra_times_rb);
    std::cout << ra.max() << " " << rb.max() << std::endl;
    EXPECT_EQ(ra | rb, ra_join_rb) << (ra | rb).min() << " ; " << (ra | rb).delta();
}

TEST_F(PolarTest, interval_operations)
{
    EXPECT_EQ(i1 * i2, i1_times_i2);
    EXPECT_EQ(i1 | i2, i1_join_i2);
    EXPECT_EQ(polar::zero, polar::zero * polar::one);
}

TEST_F(PolarTest, multiply)
{
    auto one = polar::Interval(1.);
    auto minus = polar::Interval(-1.);
    EXPECT_EQ(one * minus, minus);
    EXPECT_EQ(minus, one * minus);
}

TEST_F(PolarTest, add)
{
    auto mthree = polar::Interval(-3.);
    auto sum = polar::Interval(2.) + mthree;
    auto ref = polar::Interval(2. + (-3.));
    EXPECT_EQ(sum, ref) << sum.to_string() << " vs " << ref.to_string();
}

TEST_F(PolarTest, norm)
{
    EXPECT_EQ(i1.norm(), 3.125);
    EXPECT_EQ(i2.norm(), 0.);
}

TEST_F(PolarTest, exp_2ipi_over)
{
    auto m = polar::Interval::exp_2ipi_over(2);
    auto mref = polar::Interval(-1.);
    auto i = polar::Interval::exp_2ipi_over(4);
    EXPECT_EQ(m, mref);
    EXPECT_EQ(i, polar::Interval(ampl::i));
}