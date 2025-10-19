#include <random>
#include <algorithm>
#include <selection.h>

using diagram::Diagram;

void get_random_mergees(std::vector<Diagram *> candidates, selection::Mergees *m)
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(candidates.begin(), candidates.end(), g);
    m->a = candidates[0];
    m->b = candidates[1];
}

void get_mergees_with_max_amplitude(std::vector<Diagram *> candidates, selection::Mergees *m)
{
    throw std::runtime_error("Not implemented");
}

void get_mergees_with_min_amplitude(std::vector<Diagram *> candidates, selection::Mergees *m)
{
    throw std::runtime_error("Not implemented");
}

void get_mergees_with_max_nodes(std::vector<Diagram *> candidates, selection::Mergees *m)
{
    throw std::runtime_error("Not implemented");
}

void get_mergees_with_min_nodes(std::vector<Diagram *> candidates, selection::Mergees *m)
{
    throw std::runtime_error("Not implemented");
}

selection::Mergees selection::get_mergees_at_height(const size_t h, const Diagram *d, MergeesChoiceStrategy strategy)
{
    if (h == 0)
    {
        throw std::invalid_argument("0-height diagrams have no mergees");
    }
    struct Mergees result;
    std::vector<Diagram *> candidates = d->get_node_pointers_at_height(h);
    switch (strategy)
    {
    case RANDOM:
    {
        get_random_mergees(candidates, &result);
        break;
    }
    case MAX_AMPLITUDE:
    {
        get_mergees_with_max_amplitude(candidates, &result);
        break;
    }
    case MIN_AMPLITUDE:
    {
        get_mergees_with_min_amplitude(candidates, &result);
        break;
    }
    case MAX_NODES:
    {
        get_mergees_with_max_nodes(candidates, &result);
        break;
    }
    case MIN_NODES:
    {
        get_mergees_with_min_nodes(candidates, &result);
        break;
    }
    }
    return result;
}
