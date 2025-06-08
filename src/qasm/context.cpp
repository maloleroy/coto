#include <iostream>

#include <qasm/context.h>
#include <reduction.h>
#include <gateappliers.h>
#include <qasm/error.h>

struct action
{
    const Gate *gate;
    qubit q;
};

QasmContext::QasmContext()
{
    diagram = nullptr;
}

QasmContext::~QasmContext()
{
    delete diagram;
}

// Move constructor
QasmContext::QasmContext(QasmContext&& other) noexcept
    : storage(std::move(other.storage)),       // Move storage
      diagram(other.diagram),                 // Transfer ownership of diagram pointer
      actions(std::move(other.actions))       // Move actions vector
{
    other.diagram = nullptr; // Leave other in a valid state (no longer owns diagram)
}

// Move assignment operator
QasmContext& QasmContext::operator=(QasmContext&& other) noexcept
{
    if (this != &other) // Protect against self-assignment
    {
        // Release existing resources
        delete diagram;

        // Move resources from other
        storage = std::move(other.storage);
        diagram = other.diagram;
        actions = std::move(other.actions);

        // Leave other in a valid state
        other.diagram = nullptr;
    }
    return *this;
}

void QasmContext::apply_gate(const Gate *gate, const std::vector<varname> &qubits_names)
{
    std::vector<qubit> q;
    for (const auto &name : qubits_names)
    {
        q.push_back(storage.get_qubit(name));
    }
    apply_gate(gate, q);
}

void QasmContext::apply_gate(const Gate *gate, const std::vector<qubit> &qubits)
{
    if (gate->size() != qubits.size())
    {
        throw SizeError("Trying to apply a gate of size " + std::to_string(gate->size()) + " to " + std::to_string(qubits.size()) + " qubits");
    }

    // std::cout << "Applying gate " << gate->name << " to qubits " << qubits[0] << " "; // for debugging, will delete later
    actions.push_back(std::make_unique<struct action>(gate, qubits[0]));
}

void QasmContext::create_diagram(bool implicit)
{
    if (diagram != nullptr)
    {
        delete diagram;
        diagram = nullptr;
        std::cout << "(Deleted the previous diagram)" << std::endl;
    }
    if (implicit)
        std::cout << "(Built the diagram)" << std::endl;
    diagram = diagram::Diagram::eig0(storage.get_qubit_count());
}

void QasmContext::simulate()
{
    if (diagram == nullptr)
    {
        create_diagram(true);
    }
    for (const auto& a : actions)
    {
        if (a->gate->name == "x")
        {
            gateappliers::apply_x(diagram, a->q);
        }
        else if (a->gate->name == "h")
        {
            gateappliers::apply_h(diagram, a->q);
        }
        else if (a->gate->name == "s")
        {
            gateappliers::apply_s(diagram, a->q, a->q + 1);
        }
        else if (a->gate->name == "cx")
        {
            gateappliers::apply_cx(diagram, a->q, a->q + 1);
        }
        else if (a->gate->name[0] == 'p')
        {
            int phase = dynamic_cast<const PhaseGate *>(a->gate)->phase;
            gateappliers::apply_phase(diagram, a->q, phase);
        }
        else
        {
            std::cout << "Unimplemented gate application in context handling: " << a->gate->name << std::endl;
        }
    }
    actions.clear();
}

void QasmContext::print_list_of_actions() const
{
    for (const auto &a : actions)
    {
        std::cout << "~ " << a->gate->name << " " << a->q << std::endl;
    }
}

void QasmContext::print_evaluation()
{
    simulate();
    diagram::Evaluation eval = diagram->evaluate();
    std::cout << "\n";
    for (auto &amp : eval)
    {
        std::cout << "  ( " << amp.to_string() << " )\n";
    }
    std::cout << std::endl;
}

static void register_nodes(const diagram::Diagram *d, std::set<const diagram::Diagram *> &seen)
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

void QasmContext::print_diagram_description() const
{
    if (diagram == nullptr)
    {
        std::cout << "(null)" << std::endl;
        return;
    }
    std::set<const diagram::Diagram *> seen;
    register_nodes(diagram, seen);
    std::cout << "~ height " << diagram->height << std::endl;
    std::cout << "~ nodes " << seen.size() << std::endl;
    std::cout << "~ branches " << get_branch_count(diagram) << std::endl;
}

void QasmContext::print_run_statements_help()
{
    std::cout << "Available run statements:\n"
              << "  @build, @inst, @instantiate - create a new diagram\n"
              << "  @list, @actions - list the actions to be performed\n"
              << "  @display, @evaluate, @eval - display the evaluation of the current diagram\n"
              << "  @describe, @desc - display the description of the current diagram\n"
              << "  @help, @man, @manual - display this help message\n"
              << std::endl;
}
