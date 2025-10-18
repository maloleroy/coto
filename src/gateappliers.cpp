#include <gateappliers.h>
#include <cmath>

using diagram::Diagram;

static void assert_qubit_is_valid(Diagram *d, qubit q)
{
    if (d->height < q)
    {
        throw std::runtime_error("Invalid qubit: Trying to apply qubit " + std::to_string(q) + " to a diagram of height " + std::to_string(d->height));
    }
}

// ========== Single-qubit Pauli gates ==========

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

void gateappliers::apply_y(Diagram *d, qubit q)
{
    // Y = [[0, -i], [i, 0]]
    static const absi::Interval coeffs[] = {
        0, -ampl::i,
        ampl::i, 0};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

void gateappliers::apply_z(Diagram *d, qubit q)
{
    // Z = [[1, 0], [0, -1]]
    static const absi::Interval coeffs[] = {
        1, 0,
        0, -1};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

void gateappliers::apply_h(Diagram *d, qubit q)
{
    // H = (1/sqrt(2)) * [[1, 1], [1, -1]]
    static const absi::Interval coeffs[] = {
        ampl::inv_sqrt2, ampl::inv_sqrt2,
        ampl::inv_sqrt2, -ampl::inv_sqrt2};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

void gateappliers::apply_s(Diagram *d, qubit q)
{
    // S = [[1, 0], [0, i]]
    static const absi::Interval coeffs[] = {
        1, 0,
        0, ampl::i};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

void gateappliers::apply_sdg(Diagram *d, qubit q)
{
    // S† = [[1, 0], [0, -i]]
    static const absi::Interval coeffs[] = {
        1, 0,
        0, -ampl::i};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

void gateappliers::apply_t(Diagram *d, qubit q)
{
    // T = [[1, 0], [0, exp(i*pi/4)]]
    const auto exp_ipi4 = absi::Interval::exp_2ipi_over(8);
    const absi::Interval coeffs[] = {
        1, 0,
        0, exp_ipi4};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

void gateappliers::apply_tdg(Diagram *d, qubit q)
{
    // T† = [[1, 0], [0, exp(-i*pi/4)]]
    const auto exp_mipi4 = absi::Interval::exp_2ipi_over(-8);
    const absi::Interval coeffs[] = {
        1, 0,
        0, exp_mipi4};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

void gateappliers::apply_sx(Diagram *d, qubit q)
{
    // SX = (1/2) * [[1+i, 1-i], [1-i, 1+i]]
    const auto half = ampl::Amplitude(0.5, 0.0);
    const auto one_pi = ampl::Amplitude(0.5, 0.5);
    const auto one_mi = ampl::Amplitude(0.5, -0.5);
    const absi::Interval coeffs[] = {
        one_pi, one_mi,
        one_mi, one_pi};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

// ========== Single-qubit rotation gates ==========

void gateappliers::apply_rx(Diagram *d, qubit q, double theta)
{
    // RX(θ) = [[cos(θ/2), -i*sin(θ/2)], [-i*sin(θ/2), cos(θ/2)]]
    double half_theta = theta / 2.0;
    double cos_val = std::cos(half_theta);
    double sin_val = std::sin(half_theta);

    const absi::Interval c(cos_val);
    const absi::Interval s(ampl::Amplitude(0.0, -sin_val));
    const absi::Interval coeffs[] = {
        c, s,
        s, c};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

void gateappliers::apply_ry(Diagram *d, qubit q, double theta)
{
    // RY(θ) = [[cos(θ/2), -sin(θ/2)], [sin(θ/2), cos(θ/2)]]
    double half_theta = theta / 2.0;
    double cos_val = std::cos(half_theta);
    double sin_val = std::sin(half_theta);

    const absi::Interval c(cos_val);
    const absi::Interval s(sin_val);
    const absi::Interval ms(-sin_val);
    const absi::Interval coeffs[] = {
        c, ms,
        s, c};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

void gateappliers::apply_rz(Diagram *d, qubit q, double theta)
{
    // RZ(θ) = [[exp(-i*θ/2), 0], [0, exp(i*θ/2)]]
    const auto exp_val = absi::Interval(ampl::Amplitude(std::cos(theta / 2.0), std::sin(theta / 2.0)));
    const absi::Interval coeffs[] = {
        exp_val * ampl::Amplitude(-1.0, 0.0), 0,
        0, exp_val};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

// ========== Two-qubit gates ==========

void gateappliers::apply_swap(Diagram *d, qubit a, qubit b)
{
    // SWAP = [[1, 0, 0, 0], [0, 0, 1, 0], [0, 1, 0, 0], [0, 0, 0, 1]]
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
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

void gateappliers::apply_cx(Diagram *d, qubit a, qubit b)
{
    // CX = CNOT = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 1], [0, 0, 1, 0]]
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

void gateappliers::apply_cy(Diagram *d, qubit a, qubit b)
{
    // CY = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, -i], [0, 0, i, 0]]
    static const absi::Interval coeffs[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 0, -ampl::i,
        0, 0, ampl::i, 0};
    static const gateappliers::GateMatrix gm(2, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);
    if (b != a + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

void gateappliers::apply_cz(Diagram *d, qubit a, qubit b)
{
    // CZ = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, -1]]
    static const absi::Interval coeffs[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, -1};
    static const gateappliers::GateMatrix gm(2, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);
    if (b != a + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

void gateappliers::apply_ch(Diagram *d, qubit a, qubit b)
{
    // CH = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1/sqrt(2), 1/sqrt(2)], [0, 0, 1/sqrt(2), -1/sqrt(2)]]
    static const absi::Interval coeffs[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, ampl::inv_sqrt2, ampl::inv_sqrt2,
        0, 0, ampl::inv_sqrt2, -ampl::inv_sqrt2};
    static const gateappliers::GateMatrix gm(2, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);
    if (b != a + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

// ========== Controlled rotation gates ==========

void gateappliers::apply_cp(Diagram *d, qubit a, qubit b, double theta)
{
    // CP(θ) = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, exp(i*θ)]]
    const auto exp_itheta = absi::Interval(ampl::Amplitude(std::cos(theta), std::sin(theta)));
    const absi::Interval coeffs[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, exp_itheta};
    static const gateappliers::GateMatrix gm(2, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);
    if (b != a + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

void gateappliers::apply_crx(Diagram *d, qubit a, qubit b, double theta)
{
    // CRX(θ) = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, cos(θ/2), -i*sin(θ/2)], [0, 0, -i*sin(θ/2), cos(θ/2)]]
    double half_theta = theta / 2.0;
    double cos_val = std::cos(half_theta);
    double sin_val = std::sin(half_theta);
    const absi::Interval c(cos_val);
    const absi::Interval s(ampl::Amplitude(0.0, -sin_val));
    const absi::Interval coeffs[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, c, s,
        0, 0, s, c};
    static const gateappliers::GateMatrix gm(2, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);
    if (b != a + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

void gateappliers::apply_cry(Diagram *d, qubit a, qubit b, double theta)
{
    // CRY(θ) = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, cos(θ/2), -sin(θ/2)], [0, 0, sin(θ/2), cos(θ/2)]]
    double half_theta = theta / 2.0;
    double cos_val = std::cos(half_theta);
    double sin_val = std::sin(half_theta);
    const absi::Interval c(cos_val);
    const absi::Interval s(sin_val);
    const absi::Interval ms(-sin_val);
    const absi::Interval coeffs[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, c, ms,
        0, 0, s, c};
    static const gateappliers::GateMatrix gm(2, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);
    if (b != a + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

void gateappliers::apply_crz(Diagram *d, qubit a, qubit b, double theta)
{
    // CRZ(θ) = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, exp(-i*θ/2), 0], [0, 0, 0, exp(i*θ/2)]]
    const auto exp_itheta_half = absi::Interval(ampl::Amplitude(std::cos(theta / 2.0), std::sin(theta / 2.0)));
    const auto exp_mitheta_half = absi::Interval(ampl::Amplitude(std::cos(theta / 2.0), -std::sin(theta / 2.0)));
    const absi::Interval coeffs[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, exp_mitheta_half, 0,
        0, 0, 0, exp_itheta_half};
    static const gateappliers::GateMatrix gm(2, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);
    if (b != a + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

void gateappliers::apply_cu(Diagram *d, qubit a, qubit b, double theta, double phi, double lambda)
{
    // CU(θ, φ, λ) = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, cos(θ/2), -exp(i*λ)*sin(θ/2)], [0, 0, exp(i*φ)*sin(θ/2), exp(i*(φ+λ))*cos(θ/2)]]
    double half_theta = theta / 2.0;
    double cos_val = std::cos(half_theta);
    double sin_val = std::sin(half_theta);

    const absi::Interval c(cos_val);
    const absi::Interval s(sin_val);
    const absi::Interval exp_ilambda(ampl::Amplitude(std::cos(lambda), std::sin(lambda)));
    const absi::Interval exp_iphi(ampl::Amplitude(std::cos(phi), std::sin(phi)));
    const absi::Interval exp_iphilambda(ampl::Amplitude(std::cos(phi + lambda), std::sin(phi + lambda)));

    const absi::Interval coeffs[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, c, exp_ilambda * s * ampl::Amplitude(-1.0, 0.0),
        0, 0, exp_iphi * s, exp_iphilambda * c};
    static const gateappliers::GateMatrix gm(2, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);
    if (b != a + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

// ========== Three-qubit gates ==========

void gateappliers::apply_ccx(Diagram *d, qubit a, qubit b, qubit c)
{
    // CCX (Toffoli) = X gate on c if both a and b are 1
    // Matrix form (basis |abc>): all identity except swap |110> <-> |111>
    // Full 8x8 matrix with 1s on diagonal except positions [6,6]=0, [6,7]=1, [7,6]=1, [7,7]=0
    static const absi::Interval coeffs[] = {
        1, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 0, 0, 1, 0};
    static const gateappliers::GateMatrix gm(3, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);
    assert_qubit_is_valid(d, c);
    if (b != a + 1 || c != b + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

void gateappliers::apply_cswap(Diagram *d, qubit a, qubit b, qubit c)
{
    // CSWAP (Fredkin) = SWAP qubits b and c if a is 1
    // Full 8x8 matrix with swaps at positions (4,5), (5,4) and (6,7), (7,6)
    static const absi::Interval coeffs[] = {
        1, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 0, 0, 1, 0};
    static const gateappliers::GateMatrix gm(3, coeffs);
    assert_qubit_is_valid(d, a);
    assert_qubit_is_valid(d, b);
    assert_qubit_is_valid(d, c);
    if (b != a + 1 || c != b + 1)
    {
        throw std::runtime_error("Gate applying on non-contiguous qubits is not implemented yet");
    }
    apply_gate_matrix(d, a, gm);
}

// ========== Universal single-qubit gate ==========

void gateappliers::apply_u(Diagram *d, qubit q, double theta, double phi, double lambda)
{
    // U(θ, φ, λ) = [[cos(θ/2), -exp(i*λ)*sin(θ/2)], [exp(i*φ)*sin(θ/2), exp(i*(φ+λ))*cos(θ/2)]]
    double half_theta = theta / 2.0;
    double cos_val = std::cos(half_theta);
    double sin_val = std::sin(half_theta);

    const absi::Interval c(cos_val);
    const absi::Interval s(sin_val);
    const absi::Interval exp_ilambda(ampl::Amplitude(std::cos(lambda), std::sin(lambda)));
    const absi::Interval exp_iphi(ampl::Amplitude(std::cos(phi), std::sin(phi)));
    const absi::Interval exp_iphilambda(ampl::Amplitude(std::cos(phi + lambda), std::sin(phi + lambda)));

    const absi::Interval coeffs[] = {
        c, exp_ilambda * s * ampl::Amplitude(-1.0, 0.0),
        exp_iphi * s, exp_iphilambda * c};
    static const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

// ========== Global phase gate ==========

void gateappliers::apply_gphase(Diagram *d, double theta)
{
    // Global phase: multiply all amplitudes by exp(i*theta)
    const auto phase = absi::Interval(ampl::Amplitude(std::cos(theta), std::sin(theta)));

    for (auto &g : d->left)
    {
        g.x = phase * g.x;
    }
    for (auto &g : d->right)
    {
        g.x = phase * g.x;
    }
}

// ========== Phase gate ==========

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

// ========== Matrix application helpers ==========

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
