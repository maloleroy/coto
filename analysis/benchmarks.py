"""Safeguarded, process-isolated comparative simulator benchmarks."""

from __future__ import annotations

import argparse
import csv
import hashlib
import importlib.metadata
import json
import os
from pathlib import Path
import re
import resource
import signal
import subprocess
import sys
import tempfile
import time
import platform

from qiskit import qasm2

import circuits
from memory import find_prompt_executable
import qasm_bench
import transpile


FIELDS = (
    "engine",
    "circuit",
    "source_sha256",
    "qubits",
    "gates",
    "reduction_max_nodes",
    "repeat",
    "process_wall_seconds",
    "simulation_seconds",
    "peak_rss_bytes",
    "coto_structural_bytes",
    "timeout_seconds",
    "memory_limit_bytes",
    "status",
    "error",
)
TIME_PATTERN = re.compile(r"COTO_TIME max_rss_kib=(\d+) exit=(-?\d+)")
WORKER_PATTERN = re.compile(r"^COTO_WORKER (\{.*\})$", re.MULTILINE)
MEMORY_PATTERN = re.compile(r"~ memory usage:\s+(\d+)\s+bytes")


def benchmark_cases(profile: str) -> list[tuple[str, str]]:
    if profile == "smoke":
        return [
            ("contrived/separable_n10", circuits.separable(10)),
            ("contrived/ghz_n10", circuits.ghz(10)),
            ("small/adder_n4", qasm_bench.content("small/adder_n4")),
        ]
    if profile != "paper":
        raise ValueError(f"unknown profile: {profile}")
    cases = [
        (f"contrived/separable_n{qubits}", circuits.separable(qubits))
        for qubits in (10, 20, 30, 40, 50, 60)
    ]
    cases.extend(
        (f"contrived/ghz_n{qubits}", circuits.ghz(qubits)) for qubits in (10, 20, 30, 40, 50, 60)
    )
    cases.extend(
        (f"contrived/layered_random_n{qubits}", circuits.layered_random(qubits, 4))
        for qubits in (6, 8, 10, 12)
    )
    # A deeper, deterministic instance that is deliberately hostile to both
    # dense state vectors and exact decision-diagram sharing.
    cases.extend(
        (f"contrived/layered_random_n{qubits}_d20", circuits.layered_random(qubits, 20))
        for qubits in (20, 30)
    )
    cases.extend(circuits.fidelity_cases(include_qasmbench=True))
    unique: dict[str, str] = {}
    for name, source in cases:
        unique[name] = source
    return list(unique.items())


def _shape(source: str) -> tuple[int, int]:
    sanitized = source
    for step in (
        transpile.remove_comments,
        transpile.remove_barriers,
        transpile.remove_identities,
        transpile.remove_measurements,
        transpile.remove_classical_registers,
    ):
        sanitized = step(sanitized)
    circuit = qasm2.loads(sanitized)
    return circuit.num_qubits, len(circuit.data)


def _limit_address_space(memory_limit_bytes: int) -> None:
    resource.setrlimit(resource.RLIMIT_AS, (memory_limit_bytes, memory_limit_bytes))


def run_isolated(
    command: list[str],
    timeout: float,
    memory_limit_bytes: int,
) -> dict[str, str | int | float]:
    timed = ["/usr/bin/time", "-f", "COTO_TIME max_rss_kib=%M exit=%x", *command]
    started = time.perf_counter()
    process = subprocess.Popen(
        timed,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        start_new_session=True,
        preexec_fn=lambda: _limit_address_space(memory_limit_bytes),
    )
    try:
        stdout, stderr = process.communicate(timeout=timeout)
        status = "ok" if process.returncode == 0 else "error"
    except subprocess.TimeoutExpired:
        os.killpg(process.pid, signal.SIGKILL)
        stdout, stderr = process.communicate()
        status = "timeout"
    elapsed = time.perf_counter() - started
    time_match = TIME_PATTERN.search(stderr)
    peak_rss = int(time_match.group(1)) * 1024 if time_match else ""
    memory_markers = (
        "memoryerror",
        "bad_alloc",
        "failed to allocate",
        "cannot allocate memory",
        "insufficient memory",
        "out of memory",
    )
    if status == "error" and (
        process.returncode in (-signal.SIGKILL, 137)
        or any(marker in stderr.lower() for marker in memory_markers)
    ):
        status = "memory_limit"
    return {
        "process_wall_seconds": f"{elapsed:.6f}",
        "peak_rss_bytes": peak_rss,
        "status": status,
        "stdout": stdout,
        "stderr": stderr,
        "returncode": process.returncode,
    }


def _metadata() -> dict[str, str | int]:
    packages = ("qiskit", "qiskit-aer", "mqt-ddsim", "numpy")
    metadata: dict[str, str | int] = {name: importlib.metadata.version(name) for name in packages}
    metadata.update(
        {
            "python": platform.python_version(),
            "platform": platform.platform(),
            "processor": platform.processor(),
            "logical_cpus": os.cpu_count() or 0,
            "git_commit": subprocess.run(
                ["git", "rev-parse", "HEAD"], capture_output=True, text=True, check=True
            ).stdout.strip(),
        }
    )
    return metadata


def collect(
    output: Path,
    profile: str,
    repeats: int,
    timeout: float,
    memory_limit_bytes: int,
    coto_budgets: list[int | None],
) -> int:
    output.parent.mkdir(parents=True, exist_ok=True)
    metadata = _metadata()
    metadata.update(
        {
            "profile": profile,
            "repeats": repeats,
            "timeout_seconds": timeout,
            "memory_limit_bytes": memory_limit_bytes,
            "coto_budgets": ["exact" if budget is None else budget for budget in coto_budgets],
        }
    )
    output.with_suffix(".metadata.json").write_text(
        json.dumps(metadata, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    failures = 0
    worker = Path(__file__).with_name("benchmark_worker.py")
    with output.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=FIELDS, lineterminator="\n")
        writer.writeheader()
        with tempfile.TemporaryDirectory(prefix="coto-benchmark-") as temporary:
            temporary_path = Path(temporary)
            for circuit_id, source in benchmark_cases(profile):
                source_hash = hashlib.sha256(source.encode()).hexdigest()
                qubits, gates = _shape(source)
                source_path = temporary_path / f"{source_hash}.qasm"
                source_path.write_text(source, encoding="utf-8")
                configurations: list[tuple[str, int | None]] = [
                    ("aer", None),
                    ("mqt_ddsim", None),
                    *(("coto", budget) for budget in coto_budgets),
                ]
                for engine, budget in configurations:
                    for repeat in range(repeats):
                        row: dict[str, str | int | float] = {
                            "engine": engine,
                            "circuit": circuit_id,
                            "source_sha256": source_hash,
                            "qubits": qubits,
                            "gates": gates,
                            "reduction_max_nodes": (
                                budget
                                if budget is not None
                                else ("exact" if engine == "coto" else "")
                            ),
                            "repeat": repeat,
                            "timeout_seconds": timeout,
                            "memory_limit_bytes": memory_limit_bytes,
                            "status": "error",
                            "error": "",
                        }
                        if engine == "coto":
                            coto_source = transpile.transpile(source, budget, "@memory")
                            run_path = temporary_path / f"{source_hash}-coto-{budget}.qasm"
                            run_path.write_text(coto_source, encoding="utf-8")
                            command = [find_prompt_executable(), str(run_path)]
                        else:
                            command = [sys.executable, str(worker), engine, str(source_path)]
                        result = run_isolated(command, timeout, memory_limit_bytes)
                        row.update(
                            {
                                key: result[key]
                                for key in ("process_wall_seconds", "peak_rss_bytes", "status")
                            }
                        )
                        stdout = str(result["stdout"])
                        if result["status"] == "ok" and engine == "coto":
                            match = MEMORY_PATTERN.search(stdout)
                            if match:
                                row["coto_structural_bytes"] = int(match.group(1))
                            else:
                                row["status"] = "error"
                                row["error"] = "missing Coto structural-memory marker"
                        elif result["status"] == "ok":
                            match = WORKER_PATTERN.search(stdout)
                            if match:
                                row["simulation_seconds"] = json.loads(match.group(1))[
                                    "simulation_seconds"
                                ]
                            else:
                                row["status"] = "error"
                                row["error"] = "missing worker result marker"
                        if row["status"] != "ok":
                            failures += 1
                            if not row["error"]:
                                detail = [
                                    line
                                    for line in str(result["stderr"]).strip().splitlines()
                                    if "COTO_TIME" not in line
                                    and not line.startswith("Command exited")
                                ]
                                informative = next(
                                    (line for line in detail if "ERROR:" in line),
                                    detail[0] if detail else f"exit {result['returncode']}",
                                )
                                row["error"] = informative
                        writer.writerow(row)
                        stream.flush()
                        print(
                            f"{circuit_id} {engine} {row['reduction_max_nodes']} "
                            f"repeat={repeat}: {row['status']}"
                        )
    return failures


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path("results/benchmarks.csv"))
    parser.add_argument("--profile", choices=("smoke", "paper"), default="smoke")
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=60)
    parser.add_argument("--memory-gib", type=float, default=8)
    parser.add_argument("--coto-budget", type=int, action="append")
    parser.add_argument("--prompt", type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.repeats <= 0 or args.timeout <= 0 or args.memory_gib <= 0:
        raise SystemExit("repeats, timeout, and memory must be positive")
    budgets = args.coto_budget or [1, 4, 16]
    if any(budget <= 0 for budget in budgets):
        raise SystemExit("--coto-budget must be positive")
    if args.prompt:
        os.environ["COTO_PROMPT"] = str(args.prompt.resolve())
    failures = collect(
        args.output,
        args.profile,
        args.repeats,
        args.timeout,
        int(args.memory_gib * 1024**3),
        [None, *budgets],
    )
    return int(failures > 0)


if __name__ == "__main__":
    raise SystemExit(main())
