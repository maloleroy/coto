"""Deterministic benchmark circuits used by the analysis collectors."""

from __future__ import annotations

from collections.abc import Iterator
import random

import qasm_bench


QASMBENCH_FIDELITY_CASES = (
    "small/adder_n4",
    "small/iswap_n2",
    "small/lpn_n5",
    "small/qaoa_n3",
)


def _header(qubits: int) -> list[str]:
    return ["OPENQASM 2.0;", 'include "qelib1.inc";', f"qreg q[{qubits}];"]


def separable(qubits: int = 10) -> str:
    """A dense state vector represented by a very small exact decision diagram."""
    lines = _header(qubits)
    for qubit in range(qubits):
        lines.extend((f"h q[{qubit}];", f"rz({0.07 * (qubit + 1):.12g}) q[{qubit}];"))
    return "\n".join(lines) + "\n"


def ghz(qubits: int = 10) -> str:
    lines = _header(qubits) + ["h q[0];"]
    lines.extend(f"cx q[{qubit - 1}],q[{qubit}];" for qubit in range(1, qubits))
    return "\n".join(lines) + "\n"


def layered_random(qubits: int = 6, layers: int = 4, seed: int = 7) -> str:
    """A reproducible entangling circuit that creates nontrivial approximation pressure."""
    rng = random.Random(seed)
    lines = _header(qubits)
    for layer in range(layers):
        for qubit in range(qubits):
            theta = rng.uniform(-3.141592653589793, 3.141592653589793)
            gate = ("rx", "ry", "rz")[(layer + qubit) % 3]
            lines.append(f"{gate}({theta:.17g}) q[{qubit}];")
        offset = layer % 2
        for qubit in range(offset, qubits - 1, 2):
            lines.append(f"cx q[{qubit}],q[{qubit + 1}];")
    return "\n".join(lines) + "\n"


def fidelity_cases(include_qasmbench: bool = True) -> Iterator[tuple[str, str]]:
    yield "contrived/separable_n10", separable()
    yield "contrived/ghz_n10", ghz()
    yield "contrived/layered_random_n6", layered_random()
    yield "contrived/layered_random_n8", layered_random(8)
    if include_qasmbench:
        for circuit_id in QASMBENCH_FIDELITY_CASES:
            yield circuit_id, qasm_bench.content(circuit_id)
