#include <reduction.h>

#include <set>
#include <algorithm>

using diagram::Diagram, diagram::Branch, diagram::Branches, selection::MergeesChoiceStrategy;

static void force_merge_at_height(Diagram *d, const size_t h, MergeesChoiceStrategy strategy);

static absi::Interval child_amplitude(const Branches brs, const Diagram *possible_child);

void reduction::cut_dead_branches(Diagram *d)
{
    for (size_t i = 1; i < d->height + 1; i++)
    {
        for (auto node : d->get_node_pointers_at_height(i))
        {
            for (auto i = 0; i < node->left.size(); i++)
            {
                if ((node->left[i]).d->left.size() == 0 && (node->left[i]).d->right.size() == 0)
                {
                    node->left.erase(node->left.begin() + i);
                    delete (node->left[i]).d;
                }
            }
            for (auto i = 0; i < node->right.size(); i++)
            {
                if ((node->right[i]).d->left.size() == 0 && (node->right[i]).d->right.size() == 0)
                {
                    node->right.erase(node->right.begin() + i);
                    delete (node->right[i]).d;
                }
            }
        }
    }
}

/**
 * @brief Reduces the diagram according to the sizes in @ref{maxNodes}.
 * We use a bottom-to top algorithm, reducing each level independantly.
 * At each level i, the number of nodes is reduced by one by "fusing" two nodes until
 * there is less than `maxNodes[i]` at this level.
 */
template <size_t height>
void reduction::max_nodes_level(Diagram *d, std::array<size_t, height> maxNodes, MergeesChoiceStrategy strategy)
{
    for (size_t i = 0; i < height; i++)
    {
        while (d->count_nodes_at_height(i) > maxNodes[i])
        {
            force_merge_at_height(d, i, strategy);
        }
    }
}

template <size_t height>
static void force_merge_at_height(Diagram *d, const size_t h, MergeesChoiceStrategy strategy)
{
    auto mergees = selection::get_mergees_at_height(h, d, strategy);
    auto result = reduction::force_merge(*mergees.a, *mergees.b);
    d->replace_nodes_at_height(h, mergees.a, mergees.b, &result);
}

/// @brief An approximation algorithm for non-additive QDDs
/// We apply @ref{force_merge} to nodes of same descendance, top to down, every time it is needed.
/// @tparam height The height of the diagram we want to reduce
/// @param d The diagram we want to reduce
template <size_t height>
void algo1(Diagram *d, std::array<size_t, height> maxNodes, MergeesChoiceStrategy strategy)
{
    for (size_t i = 0; i < height; i++)
    {
        while (d->count_nodes_at_height(i) > maxNodes[i])
        {
            force_merge_at_height(d, i, strategy);
        }
    }
}

Diagram reduction::force_merge(Diagram &a, Diagram &b)
{
    if (a.height != b.height)
    {
        throw std::invalid_argument("Trying to merge diagrams with different heights");
    }
    Diagram result(a.height);
    std::set<Branch> left_a_only, left_b_only, both_left;
    for (auto &a_branch : a.left)
    {
        auto x = child_amplitude(b.left, a_branch.d);
        if (x != absi::zero)
        {
            both_left.insert({a_branch.x | x, a_branch.d});
        }
        else
        {
            left_a_only.insert(a_branch);
        }
    }
    for (auto &b_branch : a.left)
    {
        if (child_amplitude(a.left, b_branch.d) == absi::zero)
        {
            left_b_only.insert(b_branch);
        }
    }
    std::set<Branch> right_a_only, right_b_only, both_right;
    for (auto &a_branch : a.right)
    {
        auto x = child_amplitude(b.right, a_branch.d);
        if (x != absi::zero)
        {
            both_right.insert({a_branch.x | x, a_branch.d});
        }
        else
        {
            right_a_only.insert(a_branch);
        }
    }
    for (auto &b_branch : a.right)
    {
        if (child_amplitude(a.right, b_branch.d) == absi::zero)
        {
            right_b_only.insert(b_branch);
        }
    }
    left_a_only.insert(left_b_only.begin(), left_b_only.end());
    left_a_only.insert(both_left.begin(), both_left.end());
    right_a_only.insert(right_b_only.begin(), right_b_only.end());
    right_a_only.insert(both_right.begin(), both_right.end());
    for (auto &cBranch : left_a_only)
    {
        result.left.push_back(cBranch);
    }
    for (auto &cBranch : right_a_only)
    {
        result.right.push_back(cBranch);
    }
    return result;
}

static absi::Interval child_amplitude(const Branches brs, const Diagram *possible_child)
{
    for (const auto &b : brs)
    {
        if (b.d == possible_child)
        {
            return b.x;
        }
    }
    return absi::zero;
}
