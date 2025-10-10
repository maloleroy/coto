#pragma once
#include <math.h>
#include <amplitude.h>

namespace polar
{
    using ampl::Real;

    class Interval;

    class PositiveInterval
    {
        friend class Interval;

    public:
        PositiveInterval(const Real a, const Real b);

        PositiveInterval(const Real a);

        [[nodiscard]] PositiveInterval operator+(const PositiveInterval &other) const;

        [[nodiscard]] PositiveInterval operator*(const PositiveInterval &other) const;

        [[nodiscard]] PositiveInterval operator|(const PositiveInterval &other) const;

        [[nodiscard]] bool operator==(const PositiveInterval &other) const;

    protected:
        Real min;
        Real max;
    };

    class AngleInterval
    {
        friend class Interval;

    public:
        AngleInterval(Real min, Real delta);

        AngleInterval(Real a);

        static AngleInterval min_max(Real a, Real b) noexcept;

        [[nodiscard]] AngleInterval operator+(const AngleInterval &other) const noexcept;

        [[nodiscard]] AngleInterval operator|(const AngleInterval &other) const noexcept;

        [[nodiscard]] bool operator==(const AngleInterval &other) const noexcept;

        Real min() const noexcept;
        Real delta() const noexcept;
        Real max() const noexcept;

    protected:
        /// @brief The minimal value of the interval
        /// `0 < @ref min < 2` is expressed in radians to limit
        Real _min;

        /// @brief The wideness/uncertainty of the interval
        /// Values in the interval are between @ref min and @ref min + @ref delta
        /// Moreover, 0 <= @ref delta <= 2
        Real _delta;
        void set_remainder() noexcept;
    };

    class Interval
    {
    public:
        /// @brief The singleton of 0.
        Interval();

        /// @brief The singleton of a real number.
        Interval(const Real value);

        /// @brief The singleton of an amplitude.
        Interval(const ampl::Amplitude z);

        Interval(PositiveInterval mod, AngleInterval arg);

        [[nodiscard]] static Interval exp_2ipi_over(int n);

        [[nodiscard]] Interval operator+(const Interval &other) const;

        [[nodiscard]] Interval operator*(const Interval &other) const noexcept;

        [[nodiscard]] Interval operator|(const Interval &other) const noexcept;

        [[nodiscard]] bool operator==(const Interval &other) const noexcept;

        std::string to_string(bool strict = false) const noexcept;

        polar::Real norm();

    protected:
        /**
         * @brief Checks if the number is real.
         *
         * @return true if the number is real, false otherwise.
         */
        [[nodiscard]] bool is_real() const noexcept;

        [[nodiscard]] polar::Real to_real() const;

        PositiveInterval mod;

        AngleInterval arg;
    };

    const Interval zero = 0.;

    const Interval one = 1.;
};
