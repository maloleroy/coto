/**
 * @file diagram.h
 * @brief Abstract-interpreted additive diagrams
 */
#pragma once
#include <vector>
#include <limits>
#include <stdexcept>
#include <absi.h>
#include <amplitude.h>

constexpr std::size_t pwrtwo(std::size_t exponent)
{
    if (exponent >= std::numeric_limits<std::size_t>::digits)
        throw std::overflow_error("Power of two does not fit in size_t");
    return std::size_t{1} << exponent;
}

using absi::Interval;
using std::size_t;

/**
 * @brief Namespace for the diagram module
 * @details This module contains classes and functions to manipulate diagrams.
 */
namespace diagram
{
    /**
     * @brief Enumeration of sides of a diagram
     */
    enum Side
    {
        Left,
        Right
    };

    class Diagram;

    /// @brief A incoming branch (in diagrams, an arrow)
    /// @tparam height The height of the diagram the branch points to.
    struct Branch
    {
        /// @brief Abstract weight on the branch
        absi::Interval x;

        /// @brief (Link to) the destination node
        Diagram *d;

        bool operator<(const Branch &b) const
        {
            return d < b.d;
        }
    };

    using Branches = std::vector<Branch>;

    using Evaluation = ampl::PowArray<Interval>;

    /// @brief A general-purpose abstract-interpreted additive diagram
    /// @tparam height The number of levels of the diagram. Implies having `2^height`.
    class Diagram
    {
    public:
        /// @brief Create an empty diagram with no children
        Diagram(const size_t height);

        static Diagram *from_state_vector(const ampl::ConcreteState &state);

        static Diagram *eig0(const size_t height);

        /// @brief Create a random diagram
        /// Creates a random diagram. The values on the branches always have a
        /// modulus less than 1. This function is not deterministic. The distributions
        /// used to choose the number of children and the amplitudes are undefined in this
        /// interface and are left as an implementation detail that is subject to change.
        /// @return A random diagram
        static Diagram *random(const size_t height);

        /// @brief Children of side @p s
        Branches *children_of_side(Side s);

        /// @brief Evaluate the diagram
        /// @return A mathematical vector (here a std::vector) of 2^n intervals
        Evaluation evaluate();

        /// @brief Clone the diagram
        /// @return A new diagram with the same structure
        Diagram *clone() const;

        /// @brief Replace this node's children with those of an equal-height node.
        /// @details Ownership of @p replacement is consumed.
        void replace_contents(Diagram *replacement);

        /// @brief Remove direct children that represent the empty diagram.
        /// @details Height-zero nodes are terminal leaves and are never considered empty.
        void remove_dead_children();

        /// @brief Invalidate cached summaries after an in-place mutation.
        void mark_modified();

        /// @brief Add @p d to be a left child with amplitude @p x
        /// @param d The child
        /// @param x The amplitude
        void lefto(Diagram *d, const Interval &x = absi::one);

        /// @brief Add @p d to be a right child with amplitude @p x
        /// @param d The child
        /// @param x The amplitude
        void righto(Diagram *d, const Interval &x = absi::one);

        /// @brief The number of intervals contained in the evaluation
        /// @return 2 ^ @p height
        constexpr size_t size() const
        {
            return pwrtwo(height);
        }

        /// @brief The number of nodes at a given height
        size_t count_nodes_at_height(size_t h) const;

        /// @brief Get all nodes at a given height
        std::vector<Diagram *> get_node_pointers_at_height(const size_t h) const;

        /// @brief Replace nodes @p f1 and @p f2 by @p r at a given height.
        /// @details All incoming edges are redirected and ownership of @p r is consumed.
        void replace_nodes_at_height(const size_t h, Diagram *f1, Diagram *f2, Diagram *r);

        /// Reconstruct the auxiliary parent index from reachable child edges.
        void rebuild_parent_links();

        /// @brief An interval that contains all the intervals of the evaluation.
        Interval enclosure();

        /// @brief Calculate the memory usage of the diagram including all children
        /// Takes into account all sub-children diagrams but counts each node only once
        /// @return The total memory usage in bytes
        size_t memory_usage() const;

        ~Diagram();

        const size_t height;

        /// @brief Left children
        Branches left;

        /// @brief Right children
        Branches right;

    protected:
        /// @brief Populate the diagram with random values
        void populate(const size_t total_height = 0);

        /// @brief Is the data stored at node-level up-to-date
        bool is_up_to_date = false;

        void mark_parents_as_to_be_updated() const;

        /// @brief The enclosure value if `is_up_to_date` is true
        Interval cached_enclosure;

        /** @brief The parents of the node
         * @details This is used to propagate changes in the children to the parents. righto and lefto
         * should update the parents of the children.
         */
        std::vector<Diagram *> parents;

        /// @brief Forget a child
        void forget_child(Diagram *d) noexcept;

        void remove_parent(Diagram *parent) noexcept;

        void add_parent(Diagram *parent);

        void release_children() noexcept;
    };

    const size_t CHILDREN_NUMBER_AMBITION = 5;
}
