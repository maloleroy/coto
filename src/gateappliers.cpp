#include <gateappliers.h>
#include <cmath>
#include <algorithm>
#include <unordered_map>

using diagram::Diagram;

static void apply_gate_to_qubits(
    Diagram *diagram,
    const std::vector<qubit> &operands,
    const gateappliers::GateMatrix &matrix);

static void assert_qubit_is_valid(Diagram *d, qubit q)
{
    if (q >= d->height)
    {
        throw std::runtime_error("Invalid qubit: Trying to apply qubit " + std::to_string(q) + " to a diagram of height " + std::to_string(d->height));
    }
}

// ========== Single-qubit Pauli gates ==========

void gateappliers::apply_x(Diagram *d, qubit q)
{
    assert_qubit_is_valid(d, q);
    const auto target_height = d->height - q;
    for (auto *node : d->get_node_pointers_at_height(target_height))
    {
        std::swap(node->left, node->right);
        node->mark_modified();
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
    const gateappliers::GateMatrix gm(1, coeffs);
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
    const gateappliers::GateMatrix gm(1, coeffs);
    assert_qubit_is_valid(d, q);
    apply_gate_matrix(d, q, gm);
}

void gateappliers::apply_rz(Diagram *d, qubit q, double theta)
{
    // RZ(θ) = [[exp(-i*θ/2), 0], [0, exp(i*θ/2)]]
    const auto exp_mitheta = absi::Interval(ampl::Amplitude(std::cos(theta / 2.0), -std::sin(theta / 2.0)));
    const auto exp_itheta = absi::Interval(ampl::Amplitude(std::cos(theta / 2.0), std::sin(theta / 2.0)));
    const absi::Interval coeffs[] = {
        exp_mitheta, 0,
        0, exp_itheta};
    const gateappliers::GateMatrix gm(1, coeffs);
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
    apply_gate_to_qubits(d, {a, b}, gm);
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
    apply_gate_to_qubits(d, {a, b}, gm);
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
    apply_gate_to_qubits(d, {a, b}, gm);
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
    apply_gate_to_qubits(d, {a, b}, gm);
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
    apply_gate_to_qubits(d, {a, b}, gm);
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
    const gateappliers::GateMatrix gm(2, coeffs);
    apply_gate_to_qubits(d, {a, b}, gm);
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
    const gateappliers::GateMatrix gm(2, coeffs);
    apply_gate_to_qubits(d, {a, b}, gm);
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
    const gateappliers::GateMatrix gm(2, coeffs);
    apply_gate_to_qubits(d, {a, b}, gm);
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
    const gateappliers::GateMatrix gm(2, coeffs);
    apply_gate_to_qubits(d, {a, b}, gm);
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
    const gateappliers::GateMatrix gm(2, coeffs);
    apply_gate_to_qubits(d, {a, b}, gm);
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
    apply_gate_to_qubits(d, {a, b, c}, gm);
}

void gateappliers::apply_cswap(Diagram *d, qubit a, qubit b, qubit c)
{
    // CSWAP (Fredkin) = SWAP qubits b and c if a is 1
    // Full 8x8 matrix: when the first qubit is one, swap |101> and |110>.
    static const absi::Interval coeffs[] = {
        1, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1};
    static const gateappliers::GateMatrix gm(3, coeffs);
    apply_gate_to_qubits(d, {a, b, c}, gm);
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
    const gateappliers::GateMatrix gm(1, coeffs);
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
    d->mark_modified();
}

// ========== Phase gate ==========

void gateappliers::apply_phase(Diagram *d, qubit q, double theta)
{
    assert_qubit_is_valid(d, q);
    const auto phase_shift = absi::Interval(ampl::Amplitude(std::cos(theta), std::sin(theta)));
    for (auto *node : d->get_node_pointers_at_height(d->height - q))
    {
        for (auto &g : node->right)
        {
            g.x = phase_shift * g.x;
        }
        node->mark_modified();
    }
}

// ========== Matrix application helpers ==========

static bool is_zero_matrix(const gateappliers::GateMatrix &matrix)
{
    for (size_t row = 0; row < matrix.size(); ++row)
        for (size_t column = 0; column < matrix.size(); ++column)
            if (matrix(row, column) != absi::zero)
                return false;
    return true;
}

using CloneMemo = std::unordered_map<const Diagram *, Diagram *>;

static Diagram *clone_shared(const Diagram *input, CloneMemo &memo)
{
    if (input->height == 0)
        return Diagram::eig0(0);
    if (const auto found = memo.find(input); found != memo.end())
        return found->second;
    auto *output = new Diagram(input->height);
    memo.emplace(input, output);
    for (const auto &branch : input->left)
        output->lefto(clone_shared(branch.d, memo), branch.x);
    for (const auto &branch : input->right)
        output->righto(clone_shared(branch.d, memo), branch.x);
    return output;
}

static Diagram *transformed(
    const Diagram *input,
    const gateappliers::GateMatrix &matrix,
    CloneMemo &clone_memo);

static void add_transformed_branch(
    Diagram *output,
    diagram::Side side,
    const diagram::Branch &input_branch,
    const gateappliers::GateMatrix &matrix,
    CloneMemo &clone_memo)
{
    if (is_zero_matrix(matrix))
        return;

    Diagram *child = input_branch.d;
    absi::Interval weight = input_branch.x;
    if (matrix.height() == 0)
    {
        weight = matrix.value() * weight;
        child = clone_shared(input_branch.d, clone_memo);
    }
    else
    {
        child = transformed(input_branch.d, matrix, clone_memo);
    }

    if (side == diagram::Side::Left)
        output->lefto(child, weight);
    else
        output->righto(child, weight);
}

static Diagram *transformed(
    const Diagram *input,
    const gateappliers::GateMatrix &matrix,
    CloneMemo &clone_memo)
{
    if (matrix.height() == 0 || matrix.height() > input->height)
        throw std::invalid_argument("Gate matrix height is incompatible with diagram height");

    auto *output = new Diagram(input->height);
    const auto m00 = matrix.top_left();
    const auto m01 = matrix.top_right();
    const auto m10 = matrix.bottom_left();
    const auto m11 = matrix.bottom_right();
    for (const auto &branch : input->left)
    {
        add_transformed_branch(output, diagram::Side::Left, branch, m00, clone_memo);
        add_transformed_branch(output, diagram::Side::Right, branch, m10, clone_memo);
    }
    for (const auto &branch : input->right)
    {
        add_transformed_branch(output, diagram::Side::Left, branch, m01, clone_memo);
        add_transformed_branch(output, diagram::Side::Right, branch, m11, clone_memo);
    }
    return output;
}

void gateappliers::apply_gate_matrix(Diagram *diagram, qubit q, const GateMatrix &matrix)
{
    assert_qubit_is_valid(diagram, q);
    if (matrix.height() == 0 || q + matrix.height() > diagram->height)
        throw std::invalid_argument("Gate does not fit at the requested qubit");
    const auto k = diagram->height - q; // 0 <= k < d->height

    if (q == 0)
    {
        CloneMemo clone_memo;
        diagram->replace_contents(transformed(diagram, matrix, clone_memo));
        diagram->rebuild_parent_links();
        return;
    }
    for (auto &g : diagram->get_node_pointers_at_height(k))
    {
        CloneMemo clone_memo;
        g->replace_contents(transformed(g, matrix, clone_memo));
    }
    diagram->rebuild_parent_links();
}

static size_t reorder_basis_index(
    size_t sorted_index,
    const std::vector<qubit> &operands,
    const std::vector<qubit> &sorted_operands)
{
    const size_t count = operands.size();
    size_t original_index = 0;
    for (size_t original_position = 0; original_position < count; ++original_position)
    {
        const auto sorted_position = static_cast<size_t>(std::distance(
            sorted_operands.begin(),
            std::find(sorted_operands.begin(), sorted_operands.end(), operands[original_position])));
        const size_t index_bit = (sorted_index >> (count - sorted_position - 1)) & 1U;
        original_index |= index_bit << (count - original_position - 1);
    }
    return original_index;
}

static void apply_gate_to_qubits(
    Diagram *diagram,
    const std::vector<qubit> &operands,
    const gateappliers::GateMatrix &matrix)
{
    if (operands.size() != matrix.height())
        throw std::invalid_argument("Gate operand count does not match matrix height");
    for (auto operand : operands)
        assert_qubit_is_valid(diagram, operand);

    auto sorted_operands = operands;
    std::sort(sorted_operands.begin(), sorted_operands.end());
    if (std::adjacent_find(sorted_operands.begin(), sorted_operands.end()) != sorted_operands.end())
        throw std::invalid_argument("Gate operands must be distinct");

    gateappliers::GateMatrix reordered(matrix.height());
    for (size_t row = 0; row < matrix.size(); ++row)
        for (size_t column = 0; column < matrix.size(); ++column)
            reordered(row, column) = matrix(
                reorder_basis_index(row, operands, sorted_operands),
                reorder_basis_index(column, operands, sorted_operands));

    static const absi::Interval swap_coefficients[] = {
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, 1, 0, 0,
        0, 0, 0, 1};
    static const gateappliers::GateMatrix swap_matrix(2, swap_coefficients);

    std::vector<qubit> positions = sorted_operands;
    std::vector<qubit> swaps;
    const qubit first = positions.front();
    for (size_t index = 0; index < positions.size(); ++index)
    {
        const qubit destination = static_cast<qubit>(first + index);
        while (positions[index] > destination)
        {
            const qubit left = positions[index] - 1;
            gateappliers::apply_gate_matrix(diagram, left, swap_matrix);
            swaps.push_back(left);
            for (auto &position : positions)
            {
                if (position == left)
                    ++position;
                else if (position == left + 1)
                    --position;
            }
        }
    }

    gateappliers::apply_gate_matrix(diagram, first, reordered);
    for (auto iterator = swaps.rbegin(); iterator != swaps.rend(); ++iterator)
        gateappliers::apply_gate_matrix(diagram, *iterator, swap_matrix);
}
