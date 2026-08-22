import unittest

import transpile


class TranspileTest(unittest.TestCase):
    def test_qasm_bench_subset(self):
        source = """OPENQASM 2.0;
include \"qelib1.inc\";
qreg q[2];
creg c[2];
h q; // apply to the full register
rz(pi/2) q;
id q[0];
barrier q;
measure q -> c;
"""
        result = transpile.transpile(source)
        self.assertIn("OPENQASM 3.0;", result)
        self.assertIn("qubits[2] q;", result)
        self.assertIn("h q[0];", result)
        self.assertIn("h q[1];", result)
        self.assertIn("rz(pi/2) q[0];", result)
        self.assertIn("rz(pi/2) q[1];", result)
        self.assertNotIn("measure", result)
        self.assertNotIn("barrier", result)
        self.assertNotIn("@eval", result)
        self.assertTrue(result.rstrip().endswith("@memory;"))

    def test_classical_control_is_not_silently_changed(self):
        with self.assertRaisesRegex(ValueError, "Classically-controlled"):
            transpile.transpile("OPENQASM 2.0;\nif(c==1) x q[0];")

    def test_reduction_budget_is_recorded_as_runtime_directive(self):
        result = transpile.transpile("OPENQASM 2.0;\nqreg q[2];", reduction_max_nodes=3)
        self.assertIn("@reduce(3);", result)
        self.assertLess(result.index("@reduce(3);"), result.index("@memory;"))
        with self.assertRaisesRegex(ValueError, "positive"):
            transpile.transpile("OPENQASM 2.0;", reduction_max_nodes=0)


if __name__ == "__main__":
    unittest.main()
