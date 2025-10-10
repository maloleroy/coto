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

    void apply_x(Diagram *d, qubit q);

    void apply_h(Diagram *d, qubit q);

    void apply_s(Diagram *d, qubit a, qubit b);

    void apply_cx(Diagram *d, qubit a, qubit b);

    void apply_phase(Diagram *d, qubit q, int phaseDenominator);

    void apply_gate_matrix(Diagram *d, qubit q, const GateMatrix &m);
}
