#include "diagram.h"
#include <random>

using namespace diagram;
using engine = std::mt19937;

static size_t max_children_number(size_t height, size_t total_height);

static size_t min_children_number(size_t height, size_t total_height);

Diagram Diagram::random(const size_t height)
{
    Diagram d(height);
    d.populate(height);
    return d;
}

void Diagram::populate(const size_t total_height)
{
    if (height == 0)
    {
        return;
    }
    static std::random_device dev;
    static engine rng(dev());
    std::uniform_real_distribution<double> ampl_rng(0.0, 1.0);
    std::uniform_int_distribution<size_t> children_number_rng(
        min_children_number(height, total_height),
        max_children_number(height, total_height));
    for (size_t i = 0; i < children_number_rng(rng); i++)
    {
        auto *d = new Diagram(height - 1);
        d->populate(total_height ? total_height - 1 : 0);
        lefto(d, ampl_rng(rng));
    }
    for (size_t i = 0; i < children_number_rng(rng); i++)
    {
        auto *d = new Diagram(height - 1);
        d->populate(total_height ? total_height - 1 : 0);
        righto(d, ampl_rng(rng));
    }
}

static size_t max_children_number(size_t height, size_t total_height)
{
    if (total_height == 0)
    {
        return CHILDREN_NUMBER_AMBITION;
    }
    if (height < total_height / 2)
    {
        return 2 * height * CHILDREN_NUMBER_AMBITION / total_height + 1;
    }
    return 2 * (total_height - height) * CHILDREN_NUMBER_AMBITION / total_height + 1;
}

static size_t min_children_number(size_t height, size_t total_height)
{
    // Could also be max / 2
    auto half = max_children_number(height, total_height) / 2;
    return half ? half : 1;
    if (total_height == 0)
    {
        return CHILDREN_NUMBER_AMBITION;
    }
    if (height < total_height / 2)
    {
        return 2 * height / total_height + 1;
    }
    return 2 * (total_height - height) / total_height + 1;
}
