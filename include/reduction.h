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
     * @brief Reduces the number of nodes at each level of the QDD so that level i
     * contains less than (or equal) `maxNodes[i]`.
     *
     * This function reduces the number of nodes at each level of the Quantum Decision Diagram (QDD)
     * such that level i contains less than or equal to the specified maximum number of nodes `maxNodes[i]`.
     *
     * @param d The quantum diagram we want to reduce.
     * @param maxNodes A vector containing the maximum number of nodes for each level.
     * @return Nothing, the reduction is performed in-place.
     */
    template <std::size_t height>
    void max_nodes_level(Diagram *d, std::array<size_t, height> maxNodes, selection::MergeesChoiceStrategy strategy);

    /**
     * @brief Forces the merge of two diagrams.
     *
     * This function forces the merge of two diagrams `a` and `b` and returns the merged diagram.
     *
     * @param a The first diagram to be merged.
     * @param b The second diagram to be merged.
     * @return The merged diagram.
     */
    Diagram force_merge(Diagram &a, Diagram &b);
};
