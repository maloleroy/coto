import unittest

import qasm_bench


class QasmBenchTest(unittest.TestCase):
    def test_qubit_count(self):
        self.assertEqual(qasm_bench.qubit_count("small/adder_n4"), 4)
        self.assertEqual(qasm_bench.qubit_count("large/QAOA_3SAT_N100_p100"), 100)

    def test_ids_are_stable_and_resolve(self):
        identifiers = list(qasm_bench.all_ids(["small"]))
        self.assertEqual(identifiers, sorted(identifiers))
        self.assertTrue(identifiers)
        self.assertTrue(qasm_bench.file_path(identifiers[0]).is_file())

    def test_unknown_size_is_rejected(self):
        with self.assertRaises(ValueError):
            list(qasm_bench.all_ids(["tiny"]))


if __name__ == "__main__":
    unittest.main()
