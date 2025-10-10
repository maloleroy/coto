/**
 * ABStract Interpretation (AbsI)
 */

#pragma once
#include <amplitude.h>
#include <tuple>

namespace cartesian
{
    using ampl::Real;

    typedef std::tuple<ampl::Real, ampl::Real> RealInterval;

    /// @brief Represents an interval in the Cartesian coordinate system.
    class Interval
    {
    public:
        /// @brief The singleton of 0.
        Interval();

        /// @brief A single point interval.
        /// @param value The real number.

        Interval(const ampl::Real value);

        /// @brief A single point interval.
        /// @param z The complex point.
        Interval(ampl::Amplitude z);

        Interval(const RealInterval re, const RealInterval im);

        /// @brief The interval between two complex points.
        /// @param u The first complex point.
        /// @param v The second complex point.
        Interval(const ampl::Amplitude u, const ampl::Amplitude v)
            : bottom_left(u),
              top_right(v) {};

        /// @brief The interval between four real numbers.
        /// @param a The first real part number.
        /// @param b The second real part number.
        /// @param c The first imaginary part number.
        /// @param d The second imaginary part number.
        Interval(ampl::Real a, ampl::Real b, ampl::Real c, ampl::Real d)
            : bottom_left(ampl::Amplitude(a, b)),
              top_right(ampl::Amplitude(c, d)) {};

        /// @brief Tests if two intervals are equal
        /// @return True if other is equal to the interval, false otherwise.
        bool operator==(const Interval &other) const;

        /// @brief Negates the interval.
        /// @return The negated interval.
        Interval operator-() const;

        /// @brief Computes the union of two intervals.
        /// @param other The other interval.
        /// @return The union of the two intervals.
        Interval operator|(const Interval &other) const;

        /// @brief Computes the sum of two intervals.
        /// @param other The other interval.
        /// @return The sum of the two intervals.
        Interval operator+(const Interval &other) const;

        /// @brief Computes the product of an interval and a real number.
        /// @param other The real number.
        /// @return The product of the interval and the real number.
        Interval operator*(const ampl::Real &other) const;

        /// @brief Computes the product of two intervals.
        /// @param other The other interval.
        /// @return The product of the two intervals.
        Interval operator*(const Interval &other) const;

        /// @brief Computes the distance between two intervals.
        /// @param other The other interval.
        /// @return The distance between the two intervals.
        ampl::Real operator^(const Interval &other) const;

        /// @brief Checks if a complex point is contained within the interval.
        /// @param point The complex point.
        /// @return True if the point is contained within the interval, false otherwise.
        bool contains(ampl::Amplitude point) const;

        std::string to_string() const noexcept;

        /// @brief Computes the norm of the interval.
        /// @return The norm of the interval.
        ampl::Real norm() const noexcept;

    private:
        RealInterval reals() const noexcept;
        RealInterval imaginaries() const noexcept;

    protected:
        ampl::Amplitude bottom_left; /// The bottom-left complex point of the interval.
        ampl::Amplitude top_right;   /// The top-right complex point of the interval.
    };

    const cartesian::Interval zero = ampl::zero;
    const cartesian::Interval one = ampl::one;
}
