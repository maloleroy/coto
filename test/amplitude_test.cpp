#include <gtest/gtest.h>
#include <amplitude.h>

TEST(AmplitudeTest, pow2)
{
    EXPECT_EQ(pow2(0), 1);
    EXPECT_EQ(pow2(5), 32);
    EXPECT_EQ(pow2(10), 1024);
}

TEST(AmplitudeTest, random)
{
    std::vector<ampl::Amplitude> values;
    for (auto i = 0; i < 500; i++)
    {
        auto a = ampl::random();
        EXPECT_GE(abs(a), 0);
        EXPECT_LE(abs(a), 1);
        for (auto j = 0; j < i; j++)
        {
            EXPECT_NE(values.at(j), a)
                << "Got the same value" << ampl::to_string(a)
                << " at indexes " << j << " and " << i;
        }
        values.push_back(a);
    }
}

TEST(AmplitudeTest, randomize_state)
{
    ampl::ConcreteState state(6);
    EXPECT_EQ(state.size(), 1 << 6);
    EXPECT_EQ(state[0], state[1]);
    EXPECT_NO_THROW(ampl::randomize_state(state));
    for (auto i = 0; i < state.size(); i++)
    {
        for (auto j = 0; j < i; j++)
        {
            EXPECT_NE(state[i], state[j]);
        }
    }
}