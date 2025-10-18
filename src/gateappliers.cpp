#include <gateappliers.h>

using diagram::Diagram;

static void assert_qubit_is_valid(Diagram *d, qubit q)
{
    if (d->height < q)
    {
        throw std::runtime_error("Invalid qubit: Trying to apply qubit " + std::to_string(q) + " to a diagram of height " + std::to_string(d->height));
    }
}

void gateappliers::apply_x(Diagram *d, qubit q)
{
    assert_qubit_is_valid(d, q);
    if (q == 0)
    {
        std::swap(d->left, d->right);
        return;
    }
    for (auto &g : d->left)
    {
        apply_x(g.d, q - 1);
    }
    for (auto &g : d->right)
    {
        apply_x(g.d, q - 1);
    }
}

void gateappliers::apply_h(Diagram *d, qubit q)
{
    static const absi::Interval coeffs[] = {
        ampl::inv_sqrt2, ampl::inv_sqrt2,
        ampl::inv_sqrt2, -ampl::inv_sqrt2};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

void gateappliers::apply_s(Diagram *d, qubit a, qubit b)
{
    static const absi::Interval coeffs[] = {
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1};
    static const gateappliers::GateMatrix gm(2, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);

    if (b != a + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubit is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

void gateappliers::apply_cx(Diagram *d, qubit a, qubit b)
{
    static const absi::Interval coeffs[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, 1,
        0, 0, 1, 0};
    static const gateappliers::GateMatrix gm(2, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);
    if (b != a + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

void gateappliers::apply_phase(Diagram *d, qubit q, int phaseDenominator)
{
    assert_qubit_is_valid(d, q);
    const auto phase_shift = absi::Interval::exp_2ipi_over(phaseDenominator);
    for (auto &d : d->get_node_pointers_at_height(d->height - q))
    {
        for (auto &g : d->right)
        {
            g.x = phase_shift * g.x;
        }
    }
}

static void apply_single_qubit_gate_on_first_qubit(Diagram *diagram, const gateappliers::GateMatrix &matrix)
{
    if (matrix.height() != 1)
    {
        throw std::runtime_error("Invalid matrix height: " + std::to_string(matrix.height()));
    }
    auto m00 = matrix.top_left().value();
    auto m01 = matrix.top_right().value();
    auto m10 = matrix.bottom_left().value();
    auto m11 = matrix.bottom_right().value();
    auto baseLeft = diagram->left;
    for (auto &g : diagram->left)
    {
        g.x = m00 * g.x; // No need to clone
    }
    for (auto &d : diagram->right)
    {
        diagram->lefto(d.d, m01 * d.x);
        d.x = m11 * d.x; // No need to clone
    }
    for (auto &g : baseLeft)
    {
        diagram->righto(g.d->clone(), m10 * g.x);
    }
}

static diagram::Branches clone_branches(const diagram::Branches &brs)
{
    diagram::Branches cloned;
    for (const auto &g : brs)
    {
        cloned.push_back({.x = g.x, .d = g.d->clone()});
    }
    return cloned;
}

static void apply_gate_on_first_qubits(Diagram *diagram, const gateappliers::GateMatrix &matrix)
{
    if (matrix.height() == 1)
    {
        apply_single_qubit_gate_on_first_qubit(diagram, matrix);
        return;
    }

    auto m00 = matrix.top_left();
    auto m01 = matrix.top_right();
    auto m10 = matrix.bottom_left();
    auto m11 = matrix.bottom_right();
    auto clonedLeft = clone_branches(diagram->left);
    for (auto &g : diagram->left)
    {
        apply_gate_on_first_qubits(g.d, m00);
    }
    for (auto &d : diagram->right)
    {
        auto clone = d.d->clone();
        apply_gate_on_first_qubits(clone, m01);
        diagram->lefto(clone, d.x);

        apply_gate_on_first_qubits(d.d, m11);
    }
    for (auto &g : clonedLeft)
    {
        auto clone = g.d->clone();
        apply_gate_on_first_qubits(clone, m10);
        diagram->righto(clone, g.x);
    }
}

void gateappliers::apply_gate_matrix(Diagram *diagram, qubit q, const GateMatrix &matrix)
{
    assert_qubit_is_valid(diagram, q);
    const auto k = diagram->height - q; // 0 <= k < d->height

    if (q == 0)
    {
        apply_gate_on_first_qubits(diagram, matrix);
        return;
    }
    for (auto &g : diagram->get_node_pointers_at_height(k))
    {
        apply_gate_on_first_qubits(g, matrix);
    }
}
