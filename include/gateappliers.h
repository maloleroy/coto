/**
 * @file gateappliers.h
 * @brief Contains functions to apply quantum gates to a diagram
 */
#pragma once

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
    void apply_phase(Diagram *d, qubit q, double theta);

    // Matrix application helper
    void apply_gate_matrix(Diagram *d, qubit q, const GateMatrix &m);

}
