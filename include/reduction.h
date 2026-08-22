/**
 * @file reduction.h
 * @brief Reducing abstract-interpreted additive diagrams
 */
#pragma once
#include <selection.h>

/**
 * @brief A module packaging multiple reduction methods for quantum decision diagrams
 */
namespace reduction
{
    using diagram::Diagram;

    /**
     * @brief Cut dead branches from a diagram.
     *
     * @param d The quantum diagram we want to reduce.
     * @return Nothing, the reduction is performed in-place.
     */
    void cut_dead_branches(Diagram *d);

    /**
     * @brief Caps every nonterminal level of a diagram at @p max_nodes nodes.
     *
     * @param d The quantum diagram we want to reduce.
     * @param max_nodes The positive, uniform node budget.
     * @param strategy The merge-pair selection policy.
     * @return The number of node merges performed in-place.
     */
    size_t max_nodes_per_level(
        Diagram *d,
        size_t max_nodes,
        selection::MergeesChoiceStrategy strategy = selection::MIN_IMPRECISION);

    /**
     * @brief Forces the merge of two diagrams.
     *
     * This function forces the merge of two diagrams `a` and `b` and returns the merged diagram.
     *
     * @param a The first diagram to be merged.
     * @param b The second diagram to be merged.
     * @return The merged diagram.
     */
    /// @return A heap-allocated merged node owned by the caller.
    Diagram *force_merge(const Diagram &a, const Diagram &b);
};
