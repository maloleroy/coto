"""Collect Coto interval soundness and midpoint-fidelity measurements."""

from __future__ import annotations

import argparse
import csv
import hashlib
import importlib.metadata
import json
from pathlib import Path
import platform
import subprocess
import time

import circuits
from fidelity import aer_statevector, calculate_metrics
from intervals import certified_intervals_from_qasm
import transpile


FIELDS = (
    "circuit",
    "source_sha256",
    "qubits",
    "gates",
    "reduction_max_nodes",
    "duration_seconds",
    "status",
    "error",
    "amplitudes",
    "contained_amplitudes",
    "containment_rate",
    "midpoint_fidelity",
    "midpoint_l2_error",
    "midpoint_norm",
    "mean_diameter",
    "max_diameter",
    "l2_radius",
    "certified_l2_error_bound",
)


def _circuit_shape(source: str) -> tuple[int, int]:
    from qiskit import qasm2

    circuit = qasm2.loads(source)
    gates = sum(instruction.operation.name != "measure" for instruction in circuit.data)
    return circuit.num_qubits, gates


def _metadata() -> dict[str, str]:
    packages = ("qiskit", "qiskit-aer", "mqt-ddsim", "numpy")
    metadata = {name: importlib.metadata.version(name) for name in packages}
    metadata.update(
        {
            "python": platform.python_version(),
            "platform": platform.platform(),
            "git_commit": subprocess.run(
                ["git", "rev-parse", "HEAD"], capture_output=True, text=True, check=True
            ).stdout.strip(),
            "certification": "unit_norm_triangle_bound_v1",
        }
    )
    return metadata


def collect(
    output: Path,
    budgets: list[int | None],
    timeout: float,
    include_qasmbench: bool = True,
) -> int:
    output.parent.mkdir(parents=True, exist_ok=True)
    output.with_suffix(".metadata.json").write_text(
        json.dumps(_metadata(), indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    failures = 0
    with output.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=FIELDS, lineterminator="\n")
        writer.writeheader()
        for circuit_id, source in circuits.fidelity_cases(include_qasmbench):
            source_hash = hashlib.sha256(source.encode()).hexdigest()
            try:
                exact = aer_statevector(source)
                qubits, gates = _circuit_shape(source)
            except Exception as error:
                failures += 1
                writer.writerow(
                    {
                        "circuit": circuit_id,
                        "source_sha256": source_hash,
                        "status": "reference_error",
                        "error": f"{type(error).__name__}: {error}",
                    }
                )
                stream.flush()
                continue
            for budget in budgets:
                started = time.perf_counter()
                row: dict[str, str | int | float] = {
                    "circuit": circuit_id,
                    "source_sha256": source_hash,
                    "qubits": qubits,
                    "gates": gates,
                    "reduction_max_nodes": budget if budget is not None else "exact",
                    "status": "ok",
                    "error": "",
                }
                try:
                    coto_qasm = transpile.transpile(
                        source, reduction_max_nodes=budget, run_directive="@intervals"
                    )
                    enclosed = certified_intervals_from_qasm(coto_qasm, timeout=timeout)
                    row.update(
                        calculate_metrics(
                            enclosed.intervals,
                            exact,
                            certified_l2_error_bound=enclosed.l2_error_bound,
                        ).as_dict()
                    )
                except Exception as error:
                    failures += 1
                    row["status"] = "error"
                    row["error"] = f"{type(error).__name__}: {error}"
                row["duration_seconds"] = f"{time.perf_counter() - started:.6f}"
                writer.writerow(row)
                stream.flush()
                print(f"{circuit_id} budget={row['reduction_max_nodes']}: {row['status']}")
    return failures


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path("results/fidelity.csv"))
    parser.add_argument("--budget", type=int, action="append", help="Repeat for each node budget")
    parser.add_argument("--timeout", type=float, default=60)
    parser.add_argument("--no-qasmbench", action="store_true")
    parser.add_argument("--prompt", type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    budgets = args.budget or [1, 2, 4, 8, 16]
    if any(budget <= 0 for budget in budgets):
        raise SystemExit("--budget must be positive")
    if args.prompt:
        import os

        os.environ["COTO_PROMPT"] = str(args.prompt.resolve())
    failures = collect(
        args.output,
        [None, *budgets],
        args.timeout,
        include_qasmbench=not args.no_qasmbench,
    )
    return int(failures > 0)


if __name__ == "__main__":
    raise SystemExit(main())
