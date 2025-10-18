#include <iostream>

#include <qasm/context.h>
#include <reduction.h>
#include <gateappliers.h>
#include <qasm/error.h>

struct action
{
    const Gate gate;
    std::vector<qubit> qubits;
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
QasmContext::QasmContext(QasmContext &&other) noexcept
    : storage(std::move(other.storage)), // Move storage
      diagram(other.diagram),            // Transfer ownership of diagram pointer
      actions(std::move(other.actions))  // Move actions vector
{
    other.diagram = nullptr; // Leave other in a valid state (no longer owns diagram)
}

// Move assignment operator
QasmContext &QasmContext::operator=(QasmContext &&other) noexcept
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

void QasmContext::apply_gate(const Gate &gate, const std::vector<varname> &qubits_names)
{
    std::vector<qubit> q;
    for (const auto &name : qubits_names)
    {
        // Check if this is an array reference like "q[0]"
        size_t bracket_pos = name.find('[');
        if (bracket_pos != std::string::npos)
        {
            // Parse array reference
            size_t close_bracket = name.find(']');
            if (close_bracket == std::string::npos || close_bracket <= bracket_pos)
            {
                throw SyntaxError("Invalid array reference: " + name);
            }

            std::string array_name = name.substr(0, bracket_pos);
            std::string index_str = name.substr(bracket_pos + 1, close_bracket - bracket_pos - 1);

            try
            {
                size_t index = std::stoul(index_str);
                q.push_back(storage.get_qubit_array_element(array_name, index));
            }
            catch (const VariableError &e)
            {
                // Re-throw VariableError as-is
                throw;
            }
            catch (const std::exception &e)
            {
                throw SyntaxError("Invalid array index: " + index_str);
            }
        }
        else
        {
            // Regular qubit reference
            q.push_back(storage.get_qubit(name));
        }
    }
    apply_gate(gate, q);
}

void QasmContext::apply_gate(const Gate &gate, const std::vector<qubit> &qubits)
{
    if (gate.size != qubits.size())
    {
        throw SizeError("Trying to apply a gate of size " + std::to_string(gate.size) + " to " + std::to_string(qubits.size()) + " qubits");
    }

    actions.push_back(std::make_unique<struct action>(gate, qubits));
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
    for (const auto &a : actions)
    {
        // Single-qubit Pauli gates
        if (a->gate.name == "x")
        {
            gateappliers::apply_x(diagram, a->qubits[0]);
        }
        else if (a->gate.name == "y")
        {
            gateappliers::apply_y(diagram, a->qubits[0]);
        }
        else if (a->gate.name == "z")
        {
            gateappliers::apply_z(diagram, a->qubits[0]);
        }
        else if (a->gate.name == "h")
        {
            gateappliers::apply_h(diagram, a->qubits[0]);
        }
        else if (a->gate.name == "s")
        {
            gateappliers::apply_s(diagram, a->qubits[0]);
        }
        else if (a->gate.name == "sdg")
        {
            gateappliers::apply_sdg(diagram, a->qubits[0]);
        }
        else if (a->gate.name == "t")
        {
            gateappliers::apply_t(diagram, a->qubits[0]);
        }
        else if (a->gate.name == "tdg")
        {
            gateappliers::apply_tdg(diagram, a->qubits[0]);
        }
        else if (a->gate.name == "sx")
        {
            gateappliers::apply_sx(diagram, a->qubits[0]);
        }
        // Single-qubit rotation gates
        else if (a->gate.name.starts_with("rx("))
        {
            double theta = a->gate.float_parameters.empty() ? 0.0 : a->gate.float_parameters[0];
            gateappliers::apply_rx(diagram, a->qubits[0], theta);
        }
        else if (a->gate.name.starts_with("ry("))
        {
            double theta = a->gate.float_parameters.empty() ? 0.0 : a->gate.float_parameters[0];
            gateappliers::apply_ry(diagram, a->qubits[0], theta);
        }
        else if (a->gate.name.starts_with("rz("))
        {
            double theta = a->gate.float_parameters.empty() ? 0.0 : a->gate.float_parameters[0];
            gateappliers::apply_rz(diagram, a->qubits[0], theta);
        }
        // Two-qubit gates
        else if (a->gate.name == "swap")
        {
            gateappliers::apply_swap(diagram, a->qubits[0], a->qubits[1]);
        }
        else if (a->gate.name == "cx")
        {
            gateappliers::apply_cx(diagram, a->qubits[0], a->qubits[1]);
        }
        else if (a->gate.name == "cy")
        {
            gateappliers::apply_cy(diagram, a->qubits[0], a->qubits[1]);
        }
        else if (a->gate.name == "cz")
        {
            gateappliers::apply_cz(diagram, a->qubits[0], a->qubits[1]);
        }
        else if (a->gate.name == "ch")
        {
            gateappliers::apply_ch(diagram, a->qubits[0], a->qubits[1]);
        }
        // Controlled rotation gates
        else if (a->gate.name.starts_with("cp("))
        {
            double theta = a->gate.float_parameters.empty() ? 0.0 : a->gate.float_parameters[0];
            gateappliers::apply_cp(diagram, a->qubits[0], a->qubits[1], theta);
        }
        else if (a->gate.name.starts_with("crx("))
        {
            double theta = a->gate.float_parameters.empty() ? 0.0 : a->gate.float_parameters[0];
            gateappliers::apply_crx(diagram, a->qubits[0], a->qubits[1], theta);
        }
        else if (a->gate.name.starts_with("cry("))
        {
            double theta = a->gate.float_parameters.empty() ? 0.0 : a->gate.float_parameters[0];
            gateappliers::apply_cry(diagram, a->qubits[0], a->qubits[1], theta);
        }
        else if (a->gate.name.starts_with("crz("))
        {
            double theta = a->gate.float_parameters.empty() ? 0.0 : a->gate.float_parameters[0];
            gateappliers::apply_crz(diagram, a->qubits[0], a->qubits[1], theta);
        }
        else if (a->gate.name.starts_with("cu("))
        {
            double theta = a->gate.float_parameters.size() > 0 ? a->gate.float_parameters[0] : 0.0;
            double phi = a->gate.float_parameters.size() > 1 ? a->gate.float_parameters[1] : 0.0;
            double lambda = a->gate.float_parameters.size() > 2 ? a->gate.float_parameters[2] : 0.0;
            gateappliers::apply_cu(diagram, a->qubits[0], a->qubits[1], theta, phi, lambda);
        }
        // Three-qubit gates
        else if (a->gate.name == "ccx")
        {
            gateappliers::apply_ccx(diagram, a->qubits[0], a->qubits[1], a->qubits[2]);
        }
        else if (a->gate.name == "cswap")
        {
            gateappliers::apply_cswap(diagram, a->qubits[0], a->qubits[1], a->qubits[2]);
        }
        // Universal single-qubit gate
        else if (a->gate.name.starts_with("u("))
        {
            double theta = a->gate.float_parameters.size() > 0 ? a->gate.float_parameters[0] : 0.0;
            double phi = a->gate.float_parameters.size() > 1 ? a->gate.float_parameters[1] : 0.0;
            double lambda = a->gate.float_parameters.size() > 2 ? a->gate.float_parameters[2] : 0.0;
            gateappliers::apply_u(diagram, a->qubits[0], theta, phi, lambda);
        }
        // Global phase gate
        else if (a->gate.name.starts_with("gphase("))
        {
            double theta = a->gate.float_parameters.empty() ? 0.0 : a->gate.float_parameters[0];
            gateappliers::apply_gphase(diagram, theta);
        }
        // Phase gate (legacy)
        else if (a->gate.name[0] == 'p')
        {
            gateappliers::apply_phase(diagram, a->qubits[0], a->gate.parameter.value_or(1));
        }
        else
        {
            std::cout << "Unimplemented gate application in context handling: " << a->gate.name << std::endl;
        }
    }
    actions.clear();
}

void QasmContext::print_list_of_actions() const
{
    for (const auto &a : actions)
    {
        std::cout << "~ " << a->gate.name;
        for (auto q : a->qubits)
        {
            std::cout << " " << q;
        }
        std::cout << std::endl;
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

void QasmContext::print_diagram_memory_usage() const
{
    if (diagram == nullptr)
    {
        std::cout << "(null)" << std::endl;
        return;
    }
    size_t memory_bytes = diagram->memory_usage();
    double memory_kb = memory_bytes / 1024.0;
    double memory_mb = memory_kb / 1024.0;

    std::cout << "~ memory usage: " << memory_bytes << " bytes";
    if (memory_bytes >= 1024)
    {
        std::cout << " (" << memory_kb << " KB)";
    }
    if (memory_bytes >= 1024 * 1024)
    {
        std::cout << " (" << memory_mb << " MB)";
    }
    std::cout << std::endl;
}

void QasmContext::print_run_statements_help()
{
    std::cout << "Available run statements:\n"
              << "  @build, @inst, @instantiate - create a new diagram\n"
              << "  @list, @actions - list the actions to be performed\n"
              << "  @display, @evaluate, @eval - display the evaluation of the current diagram\n"
              << "  @describe, @desc - display the description of the current diagram\n"
              << "  @memory, @mem - display the memory usage of the current diagram\n"
              << "  @help, @man, @manual - display this help message\n"
              << std::endl;
}
