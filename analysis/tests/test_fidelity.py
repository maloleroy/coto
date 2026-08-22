import math
import unittest

import numpy as np

import circuits
from fidelity import aer_statevector, calculate_metrics, reverse_bits
from intervals import ComplexInterval, intervals_from_qasm
import qasm_bench
from transpile import transpile


class FidelityTest(unittest.TestCase):
    def test_bit_reversal(self):
        self.assertEqual([reverse_bits(index, 3) for index in range(8)], [0, 4, 2, 6, 1, 5, 3, 7])

    def test_exact_metrics(self):
        exact = np.asarray([1 / math.sqrt(2), 0, 0, 1j / math.sqrt(2)])
        intervals = [ComplexInterval(x.real, x.real, x.imag, x.imag) for x in exact]
        metrics = calculate_metrics(intervals, exact)
        self.assertEqual(metrics.containment_rate, 1)
        self.assertAlmostEqual(metrics.midpoint_fidelity, 1)
        self.assertAlmostEqual(metrics.midpoint_l2_error, 0)
        self.assertAlmostEqual(metrics.max_diameter, 0)

    def test_endianness_alignment(self):
        exact = np.asarray([0, 1, 0, 0], dtype=complex)  # Qiskit |01>: q0 is set.
        coto = [0, 0, 1, 0]  # Coto renders q0 as the most-significant bit.
        intervals = [ComplexInterval(value, value, 0, 0) for value in coto]
        self.assertEqual(calculate_metrics(intervals, exact).midpoint_fidelity, 1)

    def test_aer_and_coto_integration(self):
        source = """OPENQASM 2.0;
include "qelib1.inc";
qreg q[3];
h q[0];
cx q[0],q[2];
rz(0.37) q[1];
"""
        exact = aer_statevector(source)
        coto = intervals_from_qasm(transpile(source, run_directive="@intervals"), timeout=5)
        metrics = calculate_metrics(coto, exact)
        self.assertEqual(metrics.containment_rate, 1)
        self.assertAlmostEqual(metrics.midpoint_fidelity, 1, places=12)

    def test_reduction_with_duplicate_abstract_branches_remains_sound(self):
        source = qasm_bench.content("small/variational_n4")
        declarations: list[str] = []
        gates: list[str] = []
        for line in source.splitlines():
            stripped = line.strip()
            if (
                not stripped
                or stripped.startswith("//")
                or stripped.startswith(("measure", "creg", "barrier"))
            ):
                continue
            target = declarations if stripped.startswith(("OPENQASM", "include", "qreg")) else gates
            target.append(line)
        prefix = "\n".join(declarations + gates[:20]) + "\n"
        exact = aer_statevector(prefix)
        enclosed = intervals_from_qasm(
            transpile(prefix, reduction_max_nodes=1, run_directive="@intervals"), timeout=5
        )
        self.assertEqual(calculate_metrics(enclosed, exact).containment_rate, 1)

    def test_repeated_reduction_retains_a_sound_useful_midpoint(self):
        source = circuits.layered_random(6, 4)
        exact = aer_statevector(source)
        enclosed = intervals_from_qasm(
            transpile(source, reduction_max_nodes=16, run_directive="@intervals"), timeout=5
        )
        metrics = calculate_metrics(enclosed, exact)
        self.assertEqual(metrics.containment_rate, 1)
        self.assertTrue(math.isfinite(metrics.midpoint_fidelity))
        self.assertGreater(metrics.midpoint_fidelity, 0.5)
        self.assertLess(metrics.l2_radius, 100)


if __name__ == "__main__":
    unittest.main()
