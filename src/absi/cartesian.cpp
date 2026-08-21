#include <string>
#include <tuple>
#include <limits>

#include <absi/cartesian.h>

using namespace cartesian;

Interval::Interval() : bottom_left{0.}, top_right{0.} {};

Interval::Interval(RealInterval re, RealInterval im) : bottom_left{std::min(std::get<0>(re), std::get<1>(re)), std::min(std::get<0>(im), std::get<1>(im))},
                                                       top_right{std::max(std::get<0>(re), std::get<1>(re)), std::max(std::get<0>(im), std::get<1>(im))} {};

Interval::Interval(const ampl::Real value) : bottom_left{value}, top_right{value} {};

Interval::Interval(const ampl::Amplitude z) : bottom_left{z}, top_right{z} {};

Interval Interval::exp_2ipi_over(int n)
{
    ampl::Amplitude x = std::polar(1.0, 2.0 * M_PI / n);
    return Interval(x);
}

bool Interval::operator==(const Interval &other) const
{
    return (bottom_left == other.bottom_left) && (top_right == other.top_right);
}

Interval Interval::operator-() const
{
    return Interval(ampl::Amplitude(-top_right.real(), -top_right.imag()),
                    ampl::Amplitude(-bottom_left.real(), -bottom_left.imag()));
}

Interval Interval::operator|(const Interval &other) const
{
    return {ampl::Amplitude(std::min(bottom_left.real(), other.bottom_left.real()), std::min(bottom_left.imag(), other.bottom_left.imag())),
            ampl::Amplitude(std::max(top_right.real(), other.top_right.real()), std::max(top_right.imag(), other.top_right.imag()))};
}

Interval Interval::operator+(const Interval &other) const
{
    return {ampl::Amplitude(bottom_left.real() + other.bottom_left.real(), bottom_left.imag() + other.bottom_left.imag()),
            ampl::Amplitude(top_right.real() + other.top_right.real(), top_right.imag() + other.top_right.imag())};
}

Interval Interval::operator*(const ampl::Real &other) const
{
    if (other >= ampl::zero_real)
    {
        return Interval(bottom_left * other, top_right * other);
    }
    return -(*this * (-other));
}

Interval Interval::operator*(const Interval &other) const
{
    // Complex multiplication: (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
    // For intervals [z1, z2] * [w1, w2], we need to compute all corners' products
    // z1 = bottom_left, z2 = top_right, w1 = other.bottom_left, w2 = other.top_right

    ampl::Amplitude corners[] = {bottom_left, top_right,
                                 ampl::Amplitude(bottom_left.real(), top_right.imag()),
                                 ampl::Amplitude(top_right.real(), bottom_left.imag())};
    ampl::Amplitude other_corners[] = {other.bottom_left, other.top_right,
                                       ampl::Amplitude(other.bottom_left.real(), other.top_right.imag()),
                                       ampl::Amplitude(other.top_right.real(), other.bottom_left.imag())};

    ampl::Real min_real = std::numeric_limits<ampl::Real>::max();
    ampl::Real max_real = -std::numeric_limits<ampl::Real>::max();
    ampl::Real min_imag = std::numeric_limits<ampl::Real>::max();
    ampl::Real max_imag = -std::numeric_limits<ampl::Real>::max();

    // Compute all 16 products and track min/max for real and imaginary parts
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ampl::Amplitude product = corners[i] * other_corners[j];
            min_real = std::min(min_real, product.real());
            max_real = std::max(max_real, product.real());
            min_imag = std::min(min_imag, product.imag());
            max_imag = std::max(max_imag, product.imag());
        }
    }

    return Interval(ampl::Amplitude(min_real, min_imag), ampl::Amplitude(max_real, max_imag));
}

ampl::Real Interval::operator^(const Interval &other) const
{
    ampl::Real total = (*this | other).norm();
    return (total * 2) - norm() - other.norm();
}

bool Interval::contains(ampl::Amplitude z) const
{
    return (bottom_left.real() <= z.real()) &&
           (z.real() <= top_right.real()) &&
           (bottom_left.imag() <= z.imag()) &&
           (z.imag() <= top_right.imag());
}

ampl::Real Interval::norm() const noexcept
{
    return abs(top_right - bottom_left);
}

std::string Interval::to_string() const noexcept
{
    return "[" + ampl::to_string(bottom_left) + ", " + ampl::to_string(top_right) + "]";
}

RealInterval Interval::reals() const noexcept
{
    return std::make_tuple(bottom_left.real(), top_right.real());
}

RealInterval Interval::imaginaries() const noexcept
{
    return std::make_tuple(bottom_left.imag(), top_right.imag());
}
