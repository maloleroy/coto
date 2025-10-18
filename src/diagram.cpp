#include <array>
#include "diagram.h"
#include <iostream>
#include <algorithm>

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

Branches *Diagram::children_of_side(Side s)
{
    return s == Side::Right ? &left : &right;
}

void Diagram::lefto(Diagram *d, const absi::Interval &x)
{
    if (x == absi::zero)
    {
        return;
    }
    left.push_back(Branch{.x = x, .d = d});
    is_up_to_date = false;
    d->parents.push_back(this);
}

void Diagram::righto(Diagram *d, const absi::Interval &x)
{
    if (x == absi::zero)
    {
        return;
    }
    right.push_back(Branch{.x = x, .d = d});
    is_up_to_date = false;
    d->parents.push_back(this);
}

constexpr size_t Diagram::size() const
{
    return pwrtwo(height);
}

size_t Diagram::count_nodes_at_height(size_t h)
{
    return get_node_pointers_at_height(h).size();
}

// TODO: Implement this function in-place, not by copying vectors
std::vector<Diagram *> Diagram::get_node_pointers_at_height(const size_t h)
{
    std::vector<Diagram *> nodes;
    if (h > height)
    {
        throw std::invalid_argument("Height is greater than the diagram's height");
    }
    if (h == height)
    {
        nodes.push_back(this);
        return nodes;
    }
    for (Branch b : left)
    {
        nodes = merge_vectors_without_duplicates(nodes, b.d->get_node_pointers_at_height(h));
    }
    for (Branch b : right)
    {
        nodes = merge_vectors_without_duplicates(nodes, b.d->get_node_pointers_at_height(h));
    }
    return nodes;
}

void Diagram::replace_nodes_at_height(const size_t h, Diagram *f1, Diagram *f2, Diagram *r)
{
    throw std::runtime_error("Not implemented");
}

// TODO: Implement this function in O(n log n), not in O(n^2)
template <typename T>
static std::vector<T> merge_vectors_without_duplicates(const std::vector<T> a, const std::vector<T> b)
{
    std::vector<T> result;
    for (T d : a)
    {
        result.push_back(d);
    }
    for (T d : b)
    {
        if (std::find(result.begin(), result.end(), d) == result.end())
        {
            result.push_back(d);
        }
    }
    return result;
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

void Diagram::mark_parents_as_to_be_updated() const
{
    for (auto i : parents)
    {
        i->is_up_to_date = false;
        i->mark_parents_as_to_be_updated();
    }
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

Diagram::~Diagram()
{
    // Track already-deleted children to avoid double-deletion
    std::vector<Diagram *> deleted;

    for (Branch b : left)
    {
        if (b.d != leaf && std::find(deleted.begin(), deleted.end(), b.d) == deleted.end())
        {
            deleted.push_back(b.d);
            delete b.d;
        }
    }
    for (Branch b : right)
    {
        if (b.d != leaf && std::find(deleted.begin(), deleted.end(), b.d) == deleted.end())
        {
            deleted.push_back(b.d);
            delete b.d;
        }
    }
    for (Diagram *p : parents)
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
