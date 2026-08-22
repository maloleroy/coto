#include <reduction.h>

#include <unordered_map>
#include <algorithm>

using diagram::Branches, diagram::Diagram;

namespace
{
    using Weights = std::unordered_map<Diagram *, absi::Interval>;

    Weights aggregate(const Branches &branches)
    {
        Weights weights;
        for (const auto &branch : branches)
        {
            const auto found = weights.find(branch.d);
            if (found == weights.end())
                weights.emplace(branch.d, branch.x);
            else
                found->second = found->second + branch.x;
        }
        return weights;
    }

    void merge_side(Diagram *result, const Branches &a, const Branches &b, diagram::Side side)
    {
        const auto a_weights = aggregate(a);
        const auto b_weights = aggregate(b);
        std::vector<Diagram *> children;
        auto append = [&children](const Branches &branches)
        {
            for (const auto &branch : branches)
                if (std::find(children.begin(), children.end(), branch.d) == children.end())
                    children.push_back(branch.d);
        };
        append(a);
        append(b);

        for (auto *child : children)
        {
            const auto a_weight = a_weights.contains(child) ? a_weights.at(child) : absi::zero;
            const auto b_weight = b_weights.contains(child) ? b_weights.at(child) : absi::zero;
            const auto merged_weight = a_weight | b_weight;
            if (side == diagram::Side::Left)
                result->lefto(child, merged_weight);
            else
                result->righto(child, merged_weight);
        }
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

Diagram *reduction::force_merge(const Diagram &a, const Diagram &b)
{
    if (a.height != b.height)
        throw std::invalid_argument("Trying to merge diagrams with different heights");

    auto *result = new Diagram(a.height);
    merge_side(result, a.left, b.left, diagram::Side::Left);
    merge_side(result, a.right, b.right, diagram::Side::Right);
    return result;
}
