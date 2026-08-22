import unittest

import circuits
import memory
import transpile


class MemoryTest(unittest.TestCase):
    def test_parse_memory_output(self):
        self.assertEqual(memory._parse_memory_output("~ memory usage: 1234 bytes"), 1234)

    def test_invalid_output(self):
        with self.assertRaises(RuntimeError):
            memory._parse_memory_output("no measurement")

    def test_prompt_integration_without_state_vector_evaluation(self):
        qasm = "OPENQASM 3.0;\nqubit q;\nh q;\n@memory;\n"
        self.assertGreater(memory.memory_from_qasm(qasm, timeout=5), 0)

    def test_prompt_integration_with_reduction(self):
        qasm = "qubit[4] q;\n@reduce(1);\nh q[0];\nh q[1];\ncx q[0], q[2];\n@memory;\n"
        self.assertGreater(memory.memory_from_qasm(qasm, timeout=5), 0)

    def test_large_reduced_ghz_stays_compact(self):
        gates = ["h q[0];", *(f"cx q[{index - 1}], q[{index}];" for index in range(1, 30))]
        qasm = "qubit[30] q;\n" + "\n".join(gates) + "\n@reduce(1);\n@memory;\n"
        self.assertLess(memory.memory_from_qasm(qasm, timeout=2), 64 * 1024)

    def test_layered_reduced_circuit_uses_large_circuit_safeguard(self):
        qasm = transpile.transpile(circuits.layered_random(10), reduction_max_nodes=4)
        self.assertLess(memory.memory_from_qasm(qasm, timeout=2), 64 * 1024)


if __name__ == "__main__":
    unittest.main()
