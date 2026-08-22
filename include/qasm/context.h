/**
 * @file context.h
 * @brief Interface between the QASM interpreter and diagrams
 */
#pragma once
#include <qasm/gate.h>
#include <qasm/variables.h>
#include <diagram.h>
#include <optional>

struct action;

class QasmContext
{
public:
    QasmContext();
    ~QasmContext();

    // Move constructor
    QasmContext(QasmContext &&other) noexcept;
    // Move assignment operator
    QasmContext &operator=(QasmContext &&other) noexcept;

    // Explicitly delete copy constructor and copy assignment operator
    QasmContext(const QasmContext &) = delete;
    QasmContext &operator=(const QasmContext &) = delete;

    void apply_gate(const Gate &gate, const std::vector<varname> &qubits_names);

    void apply_gate(const Gate &gate, const std::vector<qubit> &qubits);

    void create_diagram(bool implicit = false);

    void print_list_of_actions() const;

    void print_evaluation();

    void print_interval_evaluation();

    void print_diagram_description();

    void print_diagram_memory_usage();

    void set_reduction_max_nodes(size_t max_nodes);

    void disable_reduction() noexcept;

    static void print_run_statements_help();

    VariableStorage storage;

private:
    void simulate();

    diagram::Diagram *diagram;

    std::vector<std::unique_ptr<struct action>> actions;

    std::optional<size_t> reduction_max_nodes;
    bool diagram_is_abstract = false;
};
