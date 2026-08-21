#include <array>
#include "diagram.h"
#include <iostream>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <functional>

using std::size_t;
using namespace diagram;

Diagram *leaf = new Diagram(0);

static Diagram *new_height_1_diagram(const ampl::ConcreteState &state);

template <typename T>
static std::vector<T> merge_vectors_without_duplicates(std::vector<T> a, std::vector<T> b);

static absi::Interval enclosure_side(Side s, Diagram &d);

Diagram::Diagram(const size_t height) : height(height)
{
}

Diagram *Diagram::eig0(const size_t height)
{
    if (height == 0)
    {
        return leaf;
    }
    auto d = new Diagram(height);
    d->lefto(Diagram::eig0(height - 1));
    return d;
}

Diagram *Diagram::from_state_vector(const ampl::ConcreteState &state)
{
    if (state.height() == 0)
    {
        return leaf;
    }
    if (state.height() == 1)
    {
        return new_height_1_diagram(state);
    }
    auto left = Diagram::from_state_vector(state.first_half());
    auto right = Diagram::from_state_vector(state.second_half());
    auto r = new Diagram(state.height());
    r->lefto(left);
    r->righto(right);
    return r;
}

Diagram *new_height_1_diagram(const ampl::ConcreteState &state)
{
    auto r = new Diagram(1);
    if (state[0] != ampl::zero)
    {
        r->lefto(leaf, state[0]);
    }
    if (state[1] != ampl::zero)
    {
        r->righto(leaf, state[1]);
    }
    return r;
}

Evaluation Diagram::evaluate()
{
    if (height == 0)
    {
        auto ev = Evaluation(0);
        ev[0] = absi::one;
        return ev;
    }
    Evaluation arr(height); // to be returned
    // used for temporary storage
    Evaluation left_array(height - 1), right_array(height - 1), tmp(height - 1);
    for (size_t i = 0; i < tmp.size(); i++)
    {
        left_array[i] = absi::zero;
        right_array[i] = absi::zero;
        tmp[i] = absi::zero;
    }
    for (const auto &l : left)
    {
        tmp = l.d->evaluate();
        for (size_t i = 0; i < left_array.size(); i++)
        {
            left_array[i] = l.x * tmp[i] + left_array[i];
        }
    }
    for (const auto &r : right)
    {
        tmp = r.d->evaluate();
        for (size_t i = 0; i < left_array.size(); i++)
        {
            right_array[i] = r.x * tmp[i] + right_array[i];
        }
    }
    for (size_t i = 0; i < arr.size(); i++)
    {
        arr[i] = i < left_array.size() ? left_array[i] : right_array[i - left_array.size()];
    }
    return arr;
}

Diagram *Diagram::clone() const
{
    if (height == 0)
    {
        return leaf;
    }
    auto d = new Diagram(height);
    for (const auto &b : left)
    {
        d->lefto(b.d->clone(), b.x);
    }
    for (const auto &b : right)
    {
        d->righto(b.d->clone(), b.x);
    }
    return d;
}

void Diagram::replace_contents(Diagram *replacement)
{
    if (replacement == nullptr || replacement->height != height)
    {
        throw std::invalid_argument("Replacement diagram must have the same height");
    }

    release_children();
    left = std::move(replacement->left);
    right = std::move(replacement->right);
    for (auto &branch : left)
    {
        branch.d->remove_parent(replacement);
        branch.d->add_parent(this);
    }
    for (auto &branch : right)
    {
        branch.d->remove_parent(replacement);
        branch.d->add_parent(this);
    }
    replacement->left.clear();
    replacement->right.clear();
    delete replacement;
    mark_modified();
}

void Diagram::remove_dead_children()
{
    std::vector<Diagram *> removed;
    auto prune = [&removed](Branches &branches)
    {
        branches.erase(std::remove_if(branches.begin(), branches.end(), [&removed](const Branch &branch)
                                      {
                                          const bool dead = branch.d->height > 0 &&
                                                            branch.d->left.empty() && branch.d->right.empty();
                                          if (dead && std::find(removed.begin(), removed.end(), branch.d) == removed.end())
                                              removed.push_back(branch.d);
                                          return dead;
                                      }),
                       branches.end());
    };

    prune(left);
    prune(right);
    if (removed.empty())
        return;

    for (auto *child : removed)
    {
        child->remove_parent(this);
        if (child->parents.empty())
            delete child;
    }
    mark_modified();
}

void Diagram::mark_modified()
{
    is_up_to_date = false;
    mark_parents_as_to_be_updated();
}

Branches *Diagram::children_of_side(Side s)
{
    return s == Side::Left ? &left : &right;
}

void Diagram::lefto(Diagram *d, const absi::Interval &x)
{
    if (x == absi::zero)
    {
        return;
    }
    left.push_back(Branch{.x = x, .d = d});
    mark_modified();
    d->add_parent(this);
}

void Diagram::righto(Diagram *d, const absi::Interval &x)
{
    if (x == absi::zero)
    {
        return;
    }
    right.push_back(Branch{.x = x, .d = d});
    mark_modified();
    d->add_parent(this);
}

size_t Diagram::count_nodes_at_height(size_t h)
{
    return get_node_pointers_at_height(h).size();
}

std::vector<Diagram *> Diagram::get_node_pointers_at_height(const size_t h) const
{
    if (h > height)
    {
        throw std::invalid_argument("Height is greater than the diagram's height");
    }
    std::vector<Diagram *> current = {const_cast<Diagram *>(this)};
    for (size_t current_height = height; current_height > h; --current_height)
    {
        std::vector<Diagram *> next;
        std::unordered_set<Diagram *> seen;
        for (auto *node : current)
        {
            for (const auto &branch : node->left)
            {
                if (seen.insert(branch.d).second)
                    next.push_back(branch.d);
            }
            for (const auto &branch : node->right)
            {
                if (seen.insert(branch.d).second)
                    next.push_back(branch.d);
            }
        }
        current = std::move(next);
    }
    return current;
}

void Diagram::replace_nodes_at_height(const size_t h, Diagram *f1, Diagram *f2, Diagram *r)
{
    throw std::runtime_error("Not implemented");
}

absi::Interval calculate_enclosure(Diagram &d)
{
    if (d.height == 0)
    {
        return absi::one;
    }
    absi::Interval l = enclosure_side(Side::Left, d);
    absi::Interval r = enclosure_side(Side::Right, d);
    return l | r;
}

absi::Interval Diagram::enclosure()
{
    if (!is_up_to_date)
    {
        cached_enclosure = calculate_enclosure(*this);
        mark_parents_as_to_be_updated();
        is_up_to_date = true;
    }
    return cached_enclosure;
}

size_t Diagram::memory_usage() const
{
    std::unordered_set<const Diagram *> visited;
    std::function<size_t(const Diagram *)> calculate = [&](const Diagram *d) -> size_t
    {
        if (visited.count(d))
        {
            return 0;
        }
        visited.insert(d);

        size_t total = sizeof(Diagram);

        // Account for the left and right branches vectors
        total += d->left.capacity() * sizeof(Branch);
        total += d->right.capacity() * sizeof(Branch);
        total += d->parents.capacity() * sizeof(Diagram *);

        // Account for each branch's interval (which is part of the Branch struct, already counted)
        // But we need to recurse into children
        for (const auto &b : d->left)
        {
            total += calculate(b.d);
        }
        for (const auto &b : d->right)
        {
            total += calculate(b.d);
        }

        return total;
    };

    return calculate(this);
}

void Diagram::mark_parents_as_to_be_updated() const
{
    std::unordered_set<Diagram *> visited;
    std::function<void(const Diagram *)> invalidate = [&](const Diagram *node)
    {
        for (auto *parent : node->parents)
        {
            if (!visited.insert(parent).second)
                continue;
            parent->is_up_to_date = false;
            invalidate(parent);
        }
    };
    invalidate(this);
}

void Diagram::forget_child(Diagram *d) noexcept
{
    right.erase(std::remove_if(right.begin(), right.end(), [d](Branch b)
                               { return b.d == d; }),
                right.end());
    left.erase(std::remove_if(left.begin(), left.end(), [d](Branch b)
                              { return b.d == d; }),
               left.end());
}

void Diagram::remove_parent(Diagram *parent) noexcept
{
    parents.erase(std::remove(parents.begin(), parents.end(), parent), parents.end());
}

void Diagram::add_parent(Diagram *parent)
{
    if (std::find(parents.begin(), parents.end(), parent) == parents.end())
        parents.push_back(parent);
}

void Diagram::release_children() noexcept
{
    std::vector<Diagram *> children;
    for (const auto &branch : left)
    {
        if (branch.d != leaf && std::find(children.begin(), children.end(), branch.d) == children.end())
            children.push_back(branch.d);
    }
    for (const auto &branch : right)
    {
        if (branch.d != leaf && std::find(children.begin(), children.end(), branch.d) == children.end())
            children.push_back(branch.d);
    }

    left.clear();
    right.clear();
    for (auto *child : children)
    {
        child->remove_parent(this);
        if (child->parents.empty())
            delete child;
    }
}

Diagram::~Diagram()
{
    release_children();
    auto former_parents = parents;
    parents.clear();
    for (Diagram *p : former_parents)
    {
        p->is_up_to_date = false;
        p->mark_parents_as_to_be_updated();
        p->forget_child(this);
    }
}

static absi::Interval enclosure_side(Side s, Diagram &d)
{
    absi::Interval rho = absi::zero;
    for (Branch b : *d.children_of_side(s))
    {
        absi::Interval i = b.x;
        auto j = b.d->enclosure();
        auto p = i * j;
        rho = rho + p;
    }
    return rho;
}
