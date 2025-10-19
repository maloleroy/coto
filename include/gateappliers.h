/**
 * @file gateappliers.h
 * @brief Contains functions to apply quantum gates to a diagram
 */

#include <diagram.h>
#include <qasm/variables.h>
#include <powmatrix.h>

namespace gateappliers
{
    using GateMatrix = PowMatrix<absi::Interval>;
    using diagram::Diagram;

    // Single-qubit gates
    void apply_x(Diagram *d, qubit q);
    void apply_y(Diagram *d, qubit q);
    void apply_z(Diagram *d, qubit q);
    void apply_h(Diagram *d, qubit q);
    void apply_s(Diagram *d, qubit q);
    void apply_sdg(Diagram *d, qubit q);
    void apply_t(Diagram *d, qubit q);
    void apply_tdg(Diagram *d, qubit q);
    void apply_sx(Diagram *d, qubit q);

    // Single-qubit rotation gates
    void apply_rx(Diagram *d, qubit q, double theta);
    void apply_ry(Diagram *d, qubit q, double theta);
    void apply_rz(Diagram *d, qubit q, double theta);

    // Two-qubit gates
    void apply_swap(Diagram *d, qubit a, qubit b);
    void apply_cx(Diagram *d, qubit a, qubit b);
    void apply_cy(Diagram *d, qubit a, qubit b);
    void apply_cz(Diagram *d, qubit a, qubit b);
    void apply_ch(Diagram *d, qubit a, qubit b);

    // Controlled rotation gates
    void apply_cp(Diagram *d, qubit a, qubit b, double theta);
    void apply_crx(Diagram *d, qubit a, qubit b, double theta);
    void apply_cry(Diagram *d, qubit a, qubit b, double theta);
    void apply_crz(Diagram *d, qubit a, qubit b, double theta);
    void apply_cu(Diagram *d, qubit a, qubit b, double theta, double phi, double lambda);

    // Three-qubit gates
    void apply_ccx(Diagram *d, qubit a, qubit b, qubit c);
    void apply_cswap(Diagram *d, qubit a, qubit b, qubit c);

    // Universal single-qubit gate
    void apply_u(Diagram *d, qubit q, double theta, double phi, double lambda);

    // Global phase
    void apply_gphase(Diagram *d, double theta);

    // Phase gate
    void apply_phase(Diagram *d, qubit q, int phaseDenominator);

    // Matrix application helper
    void apply_gate_matrix(Diagram *d, qubit q, const GateMatrix &m);

    // Internal helper for swapping qubits to make them consecutive and applying gates
    namespace internal
    {
        // Helper to perform SWAPs to make two qubits consecutive, then apply a gate function
        template <typename GateFunc>
        void apply_gate_with_swaps(Diagram *d, qubit a, qubit b, GateFunc gate_func)
        {
            if (a > b)
                std::swap(a, b);

            // Bring qubits a and b to be consecutive by swapping b towards a
            std::vector<qubit> swap_sequence;
            for (qubit i = b; i > a + 1; i--)
            {
                swap_sequence.push_back(i - 1);
                apply_swap(d, i - 1, i);
            }

            // Now a and a+1 are the qubits we want to work with
            gate_func(d, a);

            // Reverse the swaps to restore the original layout
            for (auto it = swap_sequence.rbegin(); it != swap_sequence.rend(); ++it)
            {
                apply_swap(d, *it, *it + 1);
            }
        }

        // Similar helper for three qubits
        template <typename GateFunc>
        void apply_gate_with_swaps_three(Diagram *d, qubit a, qubit b, qubit c, GateFunc gate_func)
        {
            // Sort the qubits to find min, mid, max
            std::vector<qubit> qubits = {a, b, c};
            std::sort(qubits.begin(), qubits.end());
            qubit min_q = qubits[0];
            qubit mid_q = qubits[1];
            qubit max_q = qubits[2];

            std::vector<std::pair<qubit, qubit>> swap_sequence;

            // Bring all qubits to be consecutive starting at min_q
            // First, move max_q to min_q + 2
            for (qubit i = max_q; i > min_q + 2; i--)
            {
                swap_sequence.push_back({i - 1, i});
                apply_swap(d, i - 1, i);
            }

            // Then, move mid_q to min_q + 1
            for (qubit i = mid_q; i > min_q + 1; i--)
            {
                swap_sequence.push_back({i - 1, i});
                apply_swap(d, i - 1, i);
            }

            // Now min_q, min_q+1, min_q+2 are consecutive
            gate_func(d, min_q);

            // Reverse the swaps to restore the original layout
            for (auto it = swap_sequence.rbegin(); it != swap_sequence.rend(); ++it)
            {
                apply_swap(d, it->first, it->second);
            }
        }
    }
}
