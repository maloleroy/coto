import unittest

from fidelity import aer_statevector
import circuits


class CircuitsTest(unittest.TestCase):
    def test_contrived_circuits_are_deterministic_and_valid(self):
        cases = list(circuits.fidelity_cases(include_qasmbench=False))
        self.assertEqual(len(cases), 3)
        for _, source in cases:
            self.assertGreater(len(aer_statevector(source)), 1)
        self.assertEqual(circuits.layered_random(), circuits.layered_random())


if __name__ == "__main__":
    unittest.main()
