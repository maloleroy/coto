/**
 * @file selection.h
 * @brief Contains functions and classes related to selection operations on mergees in diagrams.
 */
#pragma once
#include <diagram.h>

namespace selection
{
    using diagram::Diagram;
    /**
     * @brief Enumeration of strategies for choosing mergees.
     */
    enum MergeesChoiceStrategy
    {
        RANDOM,        /**< Randomly choose mergees. */
        MAX_AMPLITUDE, /**< Choose mergees with maximum amplitude. */
        MIN_AMPLITUDE, /**< Choose mergees with minimum amplitude. */
        MAX_NODES,     /**< Choose mergees with maximum number of nodes. */
        MIN_NODES      /**< Choose mergees with minimum number of nodes. */
    };

    /**
     * @brief Struct containing two Diagram pointers.
     * @tparam h The height of the Diagrams.
     */
    struct Mergees
    {
        Diagram *a;
        Diagram *b;

        bool operator==(const Mergees &m) const noexcept
        {
            return (a == m.a && b == m.b) || (a == m.b && b == m.a);
        }
    };

    /**
     * @brief Get the mergees at a certain height in a Diagram.
     *
     * @tparam height The height of the Diagram.
     * @tparam h The height at which to get the mergees.
     * @param d The Diagram.
     * @param strategy The strategy to use for choosing the mergees.
     * @return struct Mergees<h> The Mergees.
     */
    Mergees get_mergees_at_height(const size_t h, const Diagram *d, MergeesChoiceStrategy strategy);
};
