#include <absi/polar.h>

#include <stdexcept>
#include <numbers>
#include <iostream>

using namespace polar;

static Real argument(const ampl::Amplitude &z);

PositiveInterval::PositiveInterval(const Real a, const Real b) : min(a), max(b)
{
    if (a < 0 || b < 0)
    {
        throw std::range_error("Negative init. for positive interval");
    }
    if (a > b)
    {
        std::swap(min, max);
    }
}

PositiveInterval::PositiveInterval(const Real a) : min(a), max(a) {}

PositiveInterval PositiveInterval::operator+(const PositiveInterval &other) const
{
    return PositiveInterval(min + other.min, max + other.max);
}

PositiveInterval PositiveInterval::operator*(const PositiveInterval &other) const
{
    return PositiveInterval(min * other.min, max * other.max);
}

PositiveInterval polar::PositiveInterval::operator|(const PositiveInterval &other) const
{
    return PositiveInterval(std::min(min, other.min), std::max(max, other.max));
}

bool polar::PositiveInterval::operator==(const PositiveInterval &other) const
{
    return (min == other.min) && (max == other.max);
}

static constexpr Real TWO = 2.0;
static constexpr Real EPS = 1e-12;

static inline Real mod2(Real x) noexcept
{
    x = std::fmod(x, TWO);
    if (x < 0)
        x += TWO;
    // prefer 0 <= x < 2
    if (x >= TWO - EPS)
        x = 0.0;
    return x;
}

AngleInterval::AngleInterval(Real min_, Real delta_) : _min(mod2(min_)), _delta(delta_)
{
    // clamp delta in [0,2]
    if (_delta < 0)
        _delta = 0;
    if (_delta > TWO)
        _delta = TWO;
    if (_delta >= TWO - EPS)
    {
        // full circle canonical representation: start at 0, delta = 2
        _min = 0;
        _delta = TWO;
    }
}

AngleInterval::AngleInterval(Real a) : AngleInterval(a, 0.0) {}

AngleInterval AngleInterval::min_max(Real a, Real b) noexcept
{
    // helper if you need it; not used in operator| below
    if (a <= b)
        return AngleInterval(a, b - a);
    return AngleInterval(a, b - a + TWO);
}

[[nodiscard]] AngleInterval AngleInterval::operator+(const AngleInterval &other) const noexcept
{
    // if either is full circle -> sum is full
    if (_delta >= TWO - EPS || other._delta >= TWO - EPS)
    {
        return AngleInterval(0.0, TWO);
    }

    // Convert possibly-wrapping interval into 0..2 non-wrapping segments (1 or 2 segments)
    auto linear_segments = [](Real s, Real d)
    {
        std::vector<std::pair<Real, Real>> segs;
        if (d <= 0.0 + EPS)
        {
            // degenerate point
            segs.emplace_back(mod2(s), mod2(s));
            return segs;
        }
        Real e = s + d;
        if (e <= TWO + EPS)
        {
            segs.emplace_back(mod2(s), std::min(e, TWO));
        }
        else
        {
            // wrap
            segs.emplace_back(mod2(s), TWO);
            segs.emplace_back(0.0, e - TWO);
        }
        return segs;
    };

    auto segsA = linear_segments(_min, _delta);
    auto segsB = linear_segments(other._min, other._delta);

    // Build summed segments (on the line) for every pair
    std::vector<std::pair<Real, Real>> summed_mod_segments;

    for (auto &a : segsA)
    {
        Real a_len = a.second - a.first;
        for (auto &b : segsB)
        {
            Real b_len = b.second - b.first;
            Real sum_len = a_len + b_len;

            // If any pair-sum length >= full circle => entire circle covered
            if (sum_len >= TWO - EPS)
            {
                return AngleInterval(0.0, TWO);
            }

            // pairwise sum on real line
            Real s = a.first + b.first;   // in [0,4)
            Real e = a.second + b.second; // s + sum_len

            // map [s,e) into [0,2) possibly splitting
            // three cases: e <= 2 -> single segment [s,e]
            //            : s >= 2 -> map down by -2: [s-2, e-2]
            //            : s < 2 && e > 2 -> split [s,2) and [0, e-2)
            if (e <= TWO + EPS)
            {
                // no wrap after sum
                summed_mod_segments.emplace_back(mod2(s), std::min(e, TWO));
            }
            else if (s >= TWO - EPS)
            {
                // entirely >= 2, shift down
                summed_mod_segments.emplace_back(mod2(s - TWO), mod2(e - TWO));
            }
            else
            {
                // crosses the 2 boundary -> produces two segments
                summed_mod_segments.emplace_back(mod2(s), TWO);
                summed_mod_segments.emplace_back(0.0, e - TWO);
            }
        }
    }

    if (summed_mod_segments.empty())
    {
        // Both degenerate with zero length but code above should have created entries.
        return AngleInterval(0.0, 0.0);
    }

    // Sort and merge overlapping/adjacent segments on [0,2)
    std::sort(summed_mod_segments.begin(), summed_mod_segments.end(),
              [](auto &A, auto &B)
              {
                  if (std::fabs(A.first - B.first) > EPS)
                      return A.first < B.first;
                  return A.second < B.second;
              });

    std::vector<std::pair<Real, Real>> merged;
    for (auto &seg : summed_mod_segments)
    {
        if (merged.empty())
            merged.push_back(seg);
        else
        {
            auto &last = merged.back();
            if (seg.first <= last.second + EPS)
            {
                last.second = std::max(last.second, seg.second);
            }
            else
            {
                merged.push_back(seg);
            }
        }
    }

    // Compute covered length
    Real covered = 0.0;
    for (auto &m : merged)
        covered += (m.second - m.first);

    if (covered >= TWO - EPS)
    {
        return AngleInterval(0.0, TWO);
    }

    // If single merged segment -> done
    if (merged.size() == 1)
    {
        Real s = merged[0].first;
        Real len = merged[0].second - merged[0].first;
        return AngleInterval(s, len);
    }

    // Otherwise find largest gap (including wrap gap) and take complement
    Real max_gap = -1.0;
    Real gap_start_pos = 0.0;
    for (size_t i = 0; i < merged.size(); ++i)
    {
        Real end_i = merged[i].second;
        Real start_next = (i + 1 < merged.size()) ? merged[i + 1].first : (merged[0].first + TWO);
        Real gap = start_next - end_i;
        if (gap > max_gap)
        {
            max_gap = gap;
            // start of the minimal covering arc is the beginning of the next segment
            gap_start_pos = mod2(start_next);
        }
    }

    Real cover_len = TWO - max_gap;
    Real start_of_cover = mod2(gap_start_pos);
    return AngleInterval(start_of_cover, cover_len);
}

[[nodiscard]] AngleInterval AngleInterval::operator|(const AngleInterval &other) const noexcept
{
    // if either is full circle, return full
    if (_delta >= TWO - EPS || other._delta >= TWO - EPS)
    {
        return AngleInterval(0.0, TWO);
    }

    // convert (possibly-wrapping) intervals into non-wrapping segments on [0,2)
    std::vector<std::pair<Real, Real>> segs;
    auto add_segments = [&](Real s, Real d)
    {
        if (d <= 0.0 + EPS)
        {
            // zero-length interval: represent as small degenerate segment [s, s]
            segs.emplace_back(s, s);
            return;
        }
        Real e = s + d;
        if (e <= TWO + EPS)
        {
            // no wrap
            segs.emplace_back(s, std::min(e, TWO));
        }
        else
        {
            // wrap: [s,2) and [0, e-2)
            segs.emplace_back(s, TWO);
            segs.emplace_back(0.0, e - TWO);
        }
    };

    add_segments(_min, _delta);
    add_segments(other._min, other._delta);

    // sort by start
    std::sort(segs.begin(), segs.end(), [](auto &a, auto &b)
              {
            if (std::fabs(a.first - b.first) > EPS) return a.first < b.first;
            return a.second < b.second; });

    // merge overlapping/adjacent segments
    std::vector<std::pair<Real, Real>> merged;
    for (auto &seg : segs)
    {
        if (merged.empty())
        {
            merged.push_back(seg);
        }
        else
        {
            auto &last = merged.back();
            if (seg.first <= last.second + EPS)
            {
                // overlap or touch
                last.second = std::max(last.second, seg.second);
            }
            else
            {
                merged.push_back(seg);
            }
        }
    }

    // compute covered length
    Real covered = 0.0;
    for (auto &m : merged)
        covered += (m.second - m.first);

    // if coverage is entire circle (within epsilon) -> full
    if (covered >= TWO - EPS)
    {
        return AngleInterval(0.0, TWO);
    }

    // If single merged segment -> return it directly
    if (merged.size() == 1)
    {
        Real s = merged[0].first;
        Real len = merged[0].second - merged[0].first;
        return AngleInterval(s, len);
    }

    // find largest gap between merged segments (including wrap gap)
    Real max_gap = -1.0;
    Real gap_start_pos = 0.0; // start position of the minimal covering arc
    for (size_t i = 0; i < merged.size(); ++i)
    {
        Real end_i = merged[i].second;
        Real start_next;
        if (i + 1 < merged.size())
            start_next = merged[i + 1].first;
        else
            start_next = merged[0].first + TWO; // wrap-around
        Real gap = start_next - end_i;          // positive
        if (gap > max_gap)
        {
            max_gap = gap;
            // the minimal covering arc should start at the beginning of the next segment
            gap_start_pos = mod2(start_next);
        }
    }

    // Complement of largest gap is the minimal covering arc
    Real cover_len = TWO - max_gap;
    Real start_of_cover = mod2(gap_start_pos);

    return AngleInterval(start_of_cover, cover_len);
}

[[nodiscard]] bool AngleInterval::operator==(const AngleInterval &other) const noexcept
{
    // normalize comparison: if full circle
    if (_delta >= TWO - EPS && other._delta >= TWO - EPS)
        return true;
    return (std::fabs(mod2(_min) - mod2(other._min)) < 1e-10) &&
           (std::fabs(_delta - other._delta) < 1e-10);
}

// utility for testing & debugging
Real AngleInterval::min() const noexcept { return _min; }
Real AngleInterval::delta() const noexcept { return _delta; }
Real AngleInterval::max() const noexcept
{
    auto m = _min + _delta;
    if (m > TWO)
        m -= TWO;
    return m;
}

void AngleInterval::set_remainder() noexcept
{
    _min = mod2(_min);
    if (_delta >= TWO - EPS)
    {
        _min = 0.0;
        _delta = TWO;
    }
}

Interval::Interval(PositiveInterval mod, AngleInterval arg) : mod(mod), arg(arg) {}

polar::Interval::Interval() : mod(PositiveInterval(0.)), arg(AngleInterval(0.)) {};

polar::Interval::Interval(const polar::Real value) : mod(PositiveInterval(std::abs(value))),
                                                     arg(AngleInterval(value < 0. ? 1. : 0.)) {}

polar::Interval::Interval(const ampl::Amplitude z) : mod(PositiveInterval(std::abs(z))), arg(argument(z)) {}

static Real argument(const ampl::Amplitude &z)
{
    if (z.imag() == 0)
    {
        return z.real() >= 0 ? 0. : 1.;
    }
    if (z.real() == 1)
    {
        return z.imag() >= 0 ? .5 : 1.5;
    }
    return std::arg(z) / std::numbers::pi;
}

Interval polar::Interval::exp_2ipi_over(int n)
{
    return Interval(PositiveInterval(1.), AngleInterval(2. / n));
}

Interval polar::Interval::operator+(const Interval &other) const
{
    if (*this == zero)
    {
        return other;
    }
    if (other == zero)
    {
        return *this;
    }
    if (is_real() && other.is_real())
    {
        return Interval(to_real() + other.to_real());
    }
    throw std::logic_error("Sum of polar intervals: " + to_string() + " + " + other.to_string());
}

Interval Interval::operator*(const Interval &other) const noexcept
{
    if (*this == zero || other == zero)
    {
        return zero;
    }
    return Interval(mod * other.mod, arg + other.arg);
}

Interval polar::Interval::operator|(const Interval &other) const noexcept
{
    return Interval(mod | other.mod, arg | other.arg);
}

bool polar::Interval::operator==(const Interval &other) const noexcept
{
    return (mod == other.mod) && (arg == other.arg);
}

std::string polar::Interval::to_string(bool strict) const noexcept
{
    if (!strict)
    {
        if (is_real())
            return std::to_string(to_real());
    }
    return "{mod: " + std::to_string(mod.min) + " " + std::to_string(mod.max) + " arg: " + std::to_string(arg._min) + " " + std::to_string(arg._delta) + "}";
}

polar::Real polar::Interval::norm()
{
    return arg._delta * mod.max / 2; // TODO: better approximation
}

bool polar::Interval::is_real() const noexcept
{
    return (arg._delta == 0) && (arg._min == 0. || arg._min == 1.);
}

polar::Real polar::Interval::to_real() const
{
    if (!is_real())
    {
        throw std::logic_error("Not a real number");
    }
    return mod.min * ((arg._min == 0.) ? 1. : -1.);
}
