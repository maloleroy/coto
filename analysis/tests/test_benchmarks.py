import sys
import unittest
from pathlib import Path

import benchmarks


class BenchmarksTest(unittest.TestCase):
    def test_smoke_cases_are_stable(self):
        cases = benchmarks.benchmark_cases("smoke")
        self.assertEqual(
            [name for name, _ in cases],
            [
                "contrived/separable_n10",
                "contrived/ghz_n10",
                "small/adder_n4",
            ],
        )

    def test_paper_profile_contains_deep_adversarial_case(self):
        names = {name for name, _ in benchmarks.benchmark_cases("paper")}
        self.assertIn("contrived/layered_random_n20_d20", names)
        self.assertIn("contrived/layered_random_n30_d20", names)

    def test_isolated_runner_records_peak_rss(self):
        result = benchmarks.run_isolated(
            [sys.executable, "-c", "print('ok')"], timeout=5, memory_limit_bytes=1024**3
        )
        self.assertEqual(result["status"], "ok")
        self.assertGreater(result["peak_rss_bytes"], 0)

    def test_memory_failure_is_classified(self):
        result = benchmarks.run_isolated(
            [sys.executable, "-c", "raise MemoryError('out of memory')"],
            timeout=5,
            memory_limit_bytes=1024**3,
        )
        self.assertEqual(result["status"], "memory_limit")

    def test_workers_simulate_one_shot(self):
        qasm = (
            Path(__file__).parents[1]
            / "data"
            / "QASMBench"
            / "small"
            / "adder_n4"
            / "adder_n4.qasm"
        )
        for engine in ("aer", "mqt_ddsim"):
            result = benchmarks.run_isolated(
                [
                    sys.executable,
                    str(Path(benchmarks.__file__).with_name("benchmark_worker.py")),
                    engine,
                    str(qasm),
                ],
                timeout=10,
                memory_limit_bytes=2 * 1024**3,
            )
            self.assertEqual(result["status"], "ok", result["stderr"])


if __name__ == "__main__":
    unittest.main()
