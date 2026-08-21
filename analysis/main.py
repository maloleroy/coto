"""Collect final Coto diagram footprints for QASMBench circuits."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path
import time

import memory
import qasm_bench
import transpile


FIELDS = ("circuit", "qubits", "memory_bytes", "duration_seconds", "status", "error")


def collect(
    output: Path,
    sizes: list[str],
    max_qubits: int | None,
    limit: int | None,
    timeout: float,
) -> int:
    """Run bounded measurements and write one durable CSV row per circuit."""
    output.parent.mkdir(parents=True, exist_ok=True)
    failures = 0
    selected: list[tuple[str, int]] = []
    for circuit_id in qasm_bench.all_ids(sizes):
        qubits = qasm_bench.qubit_count(circuit_id)
        if max_qubits is None or qubits <= max_qubits:
            selected.append((circuit_id, qubits))
        if limit is not None and len(selected) >= limit:
            break

    with output.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.DictWriter(stream, fieldnames=FIELDS)
        writer.writeheader()
        for circuit_id, qubits in selected:
            started = time.perf_counter()
            row: dict[str, str | int] = {
                "circuit": circuit_id,
                "qubits": qubits,
                "memory_bytes": "",
                "duration_seconds": "",
                "status": "ok",
                "error": "",
            }
            try:
                qasm = transpile.transpile(qasm_bench.content(circuit_id))
                row["memory_bytes"] = memory.memory_from_qasm(qasm, timeout=timeout)
            except Exception as error:  # Preserve failures in the reproducibility artifact.
                failures += 1
                row["status"] = "error"
                row["error"] = f"{type(error).__name__}: {error}"
            row["duration_seconds"] = f"{time.perf_counter() - started:.6f}"
            writer.writerow(row)
            stream.flush()
            print(f"{circuit_id}: {row['status']} {row['memory_bytes']}")
    return failures


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=Path("results/memory.csv"))
    parser.add_argument("--size", action="append", choices=qasm_bench.VALID_SIZES)
    parser.add_argument("--max-qubits", type=int)
    parser.add_argument("--limit", type=int)
    parser.add_argument("--timeout", type=float, default=60.0)
    parser.add_argument("--prompt", type=Path, help="Path to the Coto prompt executable")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.prompt:
        import os

        os.environ["COTO_PROMPT"] = str(args.prompt.resolve())
    failures = collect(
        args.output,
        args.size or ["small"],
        args.max_qubits,
        args.limit,
        args.timeout,
    )
    return int(failures > 0)


if __name__ == "__main__":
    raise SystemExit(main())
