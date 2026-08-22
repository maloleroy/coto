#include <reduction.h>

#include <algorithm>
#include <cmath>

using diagram::Diagram;

namespace
{
    Diagram *uniform_ones(size_t height)
    {
        if (height == 0)
            return Diagram::eig0(0);
        auto *result = new Diagram(height);
        auto *child = uniform_ones(height - 1);
        result->lefto(child);
        result->righto(child);
        return result;
    }

    Diagram *uniform_enclosure(size_t height, const absi::Interval &enclosure)
    {
        auto *result = new Diagram(height);
        if (enclosure == absi::zero)
            return result;
        auto *child = uniform_ones(height - 1);
        result->lefto(child, enclosure);
        result->righto(child, enclosure);
        return result;
    }

}

void reduction::cut_dead_branches(Diagram *d)
{
    for (size_t height = 1; height <= d->height; ++height)
        for (auto *node : d->get_node_pointers_at_height(height))
            node->remove_dead_children();
}

size_t reduction::max_nodes_per_level(
    Diagram *d,
    size_t max_nodes,
    selection::MergeesChoiceStrategy strategy)
{
    if (d == nullptr)
        throw std::invalid_argument("Cannot reduce a null diagram");
    if (max_nodes == 0)
        throw std::invalid_argument("The node budget must be at least one");

    size_t merges = 0;
    for (size_t height = 1; height < d->height; ++height)
    {
        while (d->count_nodes_at_height(height) > max_nodes)
        {
            const auto mergees = selection::get_mergees_at_height(height, d, strategy);
            auto *result = force_merge(*mergees.a, *mergees.b);
            d->replace_nodes_at_height(height, mergees.a, mergees.b, result);
            ++merges;
        }
    }
    return merges;
}

void reduction::enclose_unitary_image(Diagram *d, size_t gate_qubits)
{
    if (d == nullptr)
        throw std::invalid_argument("Cannot transform a null diagram");
    if (gate_qubits >= sizeof(size_t) * 8)
        throw std::overflow_error("Gate arity is too large");

    const auto input = d->enclosure();
    const double max_real = std::max(std::abs(input.min_real()), std::abs(input.max_real()));
    const double max_imag = std::max(std::abs(input.min_imag()), std::abs(input.max_imag()));
    const double max_modulus = std::hypot(max_real, max_imag);
    const double row_l1_bound = std::sqrt(static_cast<double>(size_t{1} << gate_qubits));
    const double bound = max_modulus * row_l1_bound;
    const absi::Interval enclosure(
        cartesian::RealInterval{-bound, bound},
        cartesian::RealInterval{-bound, bound});
    d->replace_contents(uniform_enclosure(d->height, enclosure));
}

Diagram *reduction::force_merge(const Diagram &a, const Diagram &b)
{
    if (a.height != b.height)
        throw std::invalid_argument("Trying to merge diagrams with different heights");

    // A coefficient-wise merge can reintroduce correlations both inside an
    // additive operand and through additive parents after redirection. The
    // compact uniform enclosure is monotone under every incoming coefficient,
    // so it is the sound general join for additive diagrams.
    auto &mutable_a = const_cast<Diagram &>(a);
    auto &mutable_b = const_cast<Diagram &>(b);
    return uniform_enclosure(a.height, mutable_a.enclosure() | mutable_b.enclosure());
}
