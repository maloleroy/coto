import unittest

import memory


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


if __name__ == "__main__":
    unittest.main()
