#include <iostream>

#include <qasm/context.h>
#include <reduction.h>
#include <gateappliers.h>

struct action
{
    const Gate *gate;
    qubit q;
};

const struct action NO_ACTION = {nullptr, 0};
const struct action CLEAR_ACTION = {nullptr, 1};

static diagram::Diagram *main_diagram = nullptr;

static qubit update_qubit(bool addOne)
{
    static qubit count = 0;
    if (addOne)
    {
        count++;
    }
    return count ? count - 1 : count;
}

qubit add_qubit()
{
    return update_qubit(true);
}

std::vector<struct action> *update_action(struct action a)
{
    static std::vector<struct action> actions;
    if (a.gate == nullptr)
    {
        if (a.q == CLEAR_ACTION.q)
            actions.clear();
    }
    else
    {
        actions.push_back(a);
    }
    return &actions;
}

void apply_gate(const Gate *gate, const std::vector<qubit> &qubits)
{
    update_action({gate, qubits[0]});
}

void create_diagram(bool implicit)
{
    if (main_diagram != nullptr)
    {
        delete main_diagram;
        std::cout << "(Deleted the previous diagram)" << std::endl;
    }
    if (implicit)
        std::cout << "(Built the diagram)" << std::endl;
    main_diagram = diagram::Diagram::eig0(update_qubit(false) + 1);
}

void simulate()
{
    if (main_diagram == nullptr)
    {
        create_diagram(true);
    }
    for (struct action a : *update_action(NO_ACTION))
    {
        if (a.gate->name == "x")
        {
            gateappliers::apply_x(main_diagram, a.q);
        }
        else if (a.gate->name == "h")
        {
            gateappliers::apply_h(main_diagram, a.q);
        }
        else if (a.gate->name == "s")
        {
            gateappliers::apply_s(main_diagram, a.q, a.q + 1);
        }
        else if (a.gate->name == "cx")
        {
            gateappliers::apply_cx(main_diagram, a.q, a.q + 1);
        }
        else if (a.gate->name[0] == 'p')
        {
            int phase = dynamic_cast<const PhaseGate *>(a.gate)->phase;
            gateappliers::apply_phase(main_diagram, a.q, phase);
        }
        else
        {
            std::cout << "Unimplemented gate application in context handling: " << a.gate->name << std::endl;
        }
    }
    update_action(CLEAR_ACTION);
}

void print_list_of_actions()
{
    for (struct action a : *update_action(NO_ACTION))
    {
        std::cout << "~ " << a.gate->name << " " << a.q << std::endl;
    }
}

void print_evaluation()
{
    if (main_diagram == nullptr)
        create_diagram(true);
    diagram::Evaluation eval = main_diagram->evaluate();
    std::cout << "\n";
    for (auto &amp : eval)
    {
        std::cout << "  ( " << amp.to_string() << " )\n";
    }
    std::cout << std::endl;
}

static void register_nodes(diagram::Diagram *d, std::set<diagram::Diagram *> &seen)
{
    seen.insert(d);
    for (auto &b : d->left)
    {
        if (seen.find(b.d) == seen.end())
        {
            seen.insert(b.d);
            register_nodes(b.d, seen);
        }
    }
    for (auto &b : d->right)
    {
        if (seen.find(b.d) == seen.end())
        {
            seen.insert(b.d);
            register_nodes(b.d, seen);
        }
    }
}

static size_t get_branch_count(diagram::Diagram *d)
{
    size_t count = 0;
    for (auto &b : d->left)
    {
        count += get_branch_count(b.d);
        count++;
    }
    for (auto &b : d->right)
    {
        count += get_branch_count(b.d);
        count++;
    }
    return count;
}

void print_diagram_description()
{
    if (main_diagram == nullptr)
    {
        std::cout << "(null)" << std::endl;
        return;
    }
    std::set<diagram::Diagram *> seen;
    register_nodes(main_diagram, seen);
    std::cout << "~ height " << main_diagram->height << std::endl;
    std::cout << "~ nodes " << seen.size() << std::endl;
    std::cout << "~ branches " << get_branch_count(main_diagram) << std::endl;
}

void free_diagram()
{
    delete main_diagram;
    main_diagram = nullptr;
}

void print_run_statements_help()
{
    std::cout << "Available run statements:\n"
              << "  @build, @inst, @instantiate - create a new diagram\n"
              << "  @list, @actions - list the actions to be performed\n"
              << "  @display, @evaluate, @eval - display the evaluation of the current diagram\n"
              << "  @describe, @desc - display the description of the current diagram\n"
              << "  @help, @man, @manual - display this help message\n"
              << std::endl;
}
