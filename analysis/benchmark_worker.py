"""Isolated Python worker for third-party simulator benchmarks."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import time

from qiskit import qasm2

import transpile


def load_circuit(path: Path):
    source = path.read_text(encoding="utf-8")
    for step in (
        transpile.remove_comments,
        transpile.remove_barriers,
        transpile.remove_identities,
        transpile.remove_ifs,
        transpile.remove_measurements,
        transpile.remove_classical_registers,
    ):
        source = step(source)
    circuit = qasm2.loads(source)
    circuit.measure_all()
    return circuit


def run(engine: str, path: Path) -> dict[str, float | int | str]:
    circuit = load_circuit(path)
    started = time.perf_counter()
    if engine == "aer":
        from qiskit_aer import AerSimulator

        backend = AerSimulator(method="statevector")
    elif engine == "mqt_ddsim":
        from mqt.ddsim import DDSIMProvider

        backend = DDSIMProvider().get_backend("qasm_simulator")
    else:
        raise ValueError(f"unknown engine: {engine}")
    result = backend.run(circuit, shots=1).result()
    counts = result.get_counts()
    elapsed = time.perf_counter() - started
    if sum(counts.values()) != 1:
        raise RuntimeError("simulator returned an unexpected shot count")
    return {
        "engine": engine,
        "simulation_seconds": elapsed,
        "outcomes": len(counts),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("engine", choices=("aer", "mqt_ddsim"))
    parser.add_argument("qasm", type=Path)
    args = parser.parse_args()
    print("COTO_WORKER " + json.dumps(run(args.engine, args.qasm), sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
