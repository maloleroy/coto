#include <selection.h>

#include <algorithm>
#include <limits>
#include <random>
#include <unordered_map>

using diagram::Branches, diagram::Diagram;

namespace
{
    using Weights = std::unordered_map<const Diagram *, absi::Interval>;

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

    std::vector<const Diagram *> ordered_children(const Branches &a, const Branches &b)
    {
        std::vector<const Diagram *> children;
        auto append = [&children](const Branches &branches)
        {
            for (const auto &branch : branches)
                if (std::find(children.begin(), children.end(), branch.d) == children.end())
                    children.push_back(branch.d);
        };
        append(a);
        append(b);
        return children;
    }

    double side_widening(const Branches &a, const Branches &b)
    {
        const auto a_weights = aggregate(a);
        const auto b_weights = aggregate(b);
        double widening = 0.0;
        for (const auto *child : ordered_children(a, b))
        {
            const auto a_weight = a_weights.contains(child) ? a_weights.at(child) : absi::zero;
            const auto b_weight = b_weights.contains(child) ? b_weights.at(child) : absi::zero;
            widening += a_weight ^ b_weight;
        }
        return widening;
    }

    double merge_widening(const Diagram *a, const Diagram *b)
    {
        return side_widening(a->left, b->left) + side_widening(a->right, b->right);
    }

    size_t reachable_nodes(Diagram *diagram)
    {
        size_t count = 0;
        for (size_t height = 0; height <= diagram->height; ++height)
            count += diagram->count_nodes_at_height(height);
        return count;
    }

    template <typename Metric>
    selection::Mergees choose_extreme(
        const std::vector<Diagram *> &candidates,
        Metric metric,
        bool maximum)
    {
        auto ordered = candidates;
        std::stable_sort(ordered.begin(), ordered.end(), [&](Diagram *a, Diagram *b)
                         { return maximum ? metric(a) > metric(b) : metric(a) < metric(b); });
        return {ordered[0], ordered[1]};
    }
}

selection::Mergees selection::get_mergees_at_height(
    const size_t h,
    const Diagram *d,
    MergeesChoiceStrategy strategy)
{
    if (h == 0)
        throw std::invalid_argument("Height-zero diagrams have no mergees");

    auto candidates = d->get_node_pointers_at_height(h);
    if (candidates.size() < 2)
        throw std::invalid_argument("At least two nodes are required to select mergees");

    switch (strategy)
    {
    case RANDOM:
    {
        static std::mt19937 generator(0);
        std::shuffle(candidates.begin(), candidates.end(), generator);
        return {candidates[0], candidates[1]};
    }
    case MAX_AMPLITUDE:
        return choose_extreme(candidates, [](Diagram *candidate)
                              { return candidate->enclosure() ^ absi::zero; },
                              true);
    case MIN_AMPLITUDE:
        return choose_extreme(candidates, [](Diagram *candidate)
                              { return candidate->enclosure() ^ absi::zero; },
                              false);
    case MAX_NODES:
        return choose_extreme(candidates, reachable_nodes, true);
    case MIN_NODES:
        return choose_extreme(candidates, reachable_nodes, false);
    case MIN_IMPRECISION:
    {
        selection::Mergees best{candidates[0], candidates[1]};
        double best_cost = std::numeric_limits<double>::infinity();
        for (size_t i = 0; i < candidates.size(); ++i)
        {
            for (size_t j = i + 1; j < candidates.size(); ++j)
            {
                const double cost = merge_widening(candidates[i], candidates[j]);
                if (cost < best_cost)
                {
                    best = {candidates[i], candidates[j]};
                    best_cost = cost;
                }
            }
        }
        return best;
    }
    }
    throw std::invalid_argument("Unknown merge selection strategy");
}
