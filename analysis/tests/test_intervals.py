import math
import unittest

from intervals import (
    ComplexInterval,
    intervals_from_qasm,
    parse_certified_interval_output,
    parse_interval_output,
)


class IntervalsTest(unittest.TestCase):
    def test_geometry(self):
        interval = ComplexInterval(-1, 3, -2, 4)
        self.assertEqual(interval.midpoint, 1 + 1j)
        self.assertAlmostEqual(interval.radius, math.hypot(2, 3))
        self.assertTrue(interval.contains(3 + 4j))
        self.assertFalse(interval.contains(3.1 + 4j))

    def test_parse(self):
        output = """noise
~ intervals-v1 2
~ interval 0 0 1 -0.5 0.5
~ interval 1 -1 0 2 3
~ intervals-end
"""
        parsed = parse_interval_output(output)
        self.assertEqual(parsed[0], ComplexInterval(0, 1, -0.5, 0.5))
        self.assertEqual(parsed[1], ComplexInterval(-1, 0, 2, 3))

    def test_global_certificate(self):
        output = """~ intervals-v1 1
~ approximation-l2-error-bound 1.25
~ interval 0 -1 1 -1 1
~ intervals-end
"""
        parsed = parse_certified_interval_output(output)
        self.assertEqual(parsed.l2_error_bound, 1.25)
        self.assertEqual(len(parsed.intervals), 1)

    def test_rejects_incomplete_or_duplicate_output(self):
        with self.assertRaises(RuntimeError):
            parse_interval_output("~ intervals-v1 1\n~ interval 0 0 0 0 0")
        with self.assertRaises(RuntimeError):
            parse_interval_output(
                "~ intervals-v1 1\n~ interval 0 0 0 0 0\n~ interval 0 0 0 0 0\n~ intervals-end"
            )

    def test_prompt_integration(self):
        intervals = intervals_from_qasm(
            "OPENQASM 3.0;\nqubit[2] q;\nh q[0];\n@intervals;\n", timeout=5
        )
        self.assertEqual(len(intervals), 4)
        self.assertTrue(intervals[0].contains(1 / math.sqrt(2)))
        self.assertTrue(intervals[2].contains(1 / math.sqrt(2)))


if __name__ == "__main__":
    unittest.main()
