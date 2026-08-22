"""Soundness and midpoint-fidelity metrics against Qiskit Aer."""

from __future__ import annotations

from dataclasses import asdict, dataclass
import math

import numpy as np
from qiskit import qasm2
from qiskit_aer.quantum_info import AerStatevector

from intervals import ComplexInterval
import transpile


@dataclass(frozen=True)
class FidelityMetrics:
    amplitudes: int
    contained_amplitudes: int
    containment_rate: float
    midpoint_fidelity: float
    midpoint_l2_error: float
    midpoint_norm: float
    mean_diameter: float
    max_diameter: float
    l2_radius: float
    certified_l2_error_bound: float

    def as_dict(self) -> dict[str, int | float]:
        return asdict(self)


def aer_statevector(qasm: str) -> np.ndarray:
    """Compute an exact-reference state vector with Aer (double precision)."""
    # Some QASMBench files contain malformed, unused measurement targets. Remove
    # only semantically inert syntax before asking Qiskit to parse the circuit.
    for step in (
        transpile.remove_comments,
        transpile.remove_barriers,
        transpile.remove_identities,
        transpile.remove_ifs,
        transpile.remove_measurements,
        transpile.remove_classical_registers,
    ):
        qasm = step(qasm)
    circuit = qasm2.loads(qasm)
    # AerStatevector(circuit) initializes from the circuit definition and may
    # canonicalize its global phase. Evolution preserves the physical unitary's
    # phase, which matters for component-wise interval containment.
    initial = AerStatevector.from_int(0, dims=2**circuit.num_qubits)
    return np.asarray(initial.evolve(circuit).data, dtype=np.complex128)


def reverse_bits(index: int, width: int) -> int:
    """Convert Coto's MSB-first state index to Qiskit's LSB-first index."""
    result = 0
    for _ in range(width):
        result = (result << 1) | (index & 1)
        index >>= 1
    return result


def align_to_qiskit(intervals: list[ComplexInterval]) -> list[ComplexInterval]:
    count = len(intervals)
    if count == 0 or count & (count - 1):
        raise ValueError("interval count must be a nonzero power of two")
    qubits = count.bit_length() - 1
    return [intervals[reverse_bits(index, qubits)] for index in range(count)]


def calculate_metrics(
    intervals: list[ComplexInterval],
    exact: np.ndarray,
    containment_tolerance: float = 1e-10,
    certified_l2_error_bound: float = 0.0,
) -> FidelityMetrics:
    """Compare sound enclosures and their midpoint approximation to a reference."""
    aligned = align_to_qiskit(intervals)
    if len(aligned) != len(exact):
        raise ValueError(f"dimension mismatch: {len(aligned)} intervals, {len(exact)} amplitudes")
    midpoint = np.asarray([interval.midpoint for interval in aligned], dtype=np.complex128)
    radii = np.asarray([interval.radius for interval in aligned], dtype=float)
    diameters = 2 * radii
    contained = sum(
        interval.contains(value, containment_tolerance)
        for interval, value in zip(aligned, exact, strict=True)
    )
    norm = float(np.linalg.norm(midpoint))
    exact_norm = float(np.linalg.norm(exact))
    if norm == 0 or exact_norm == 0:
        fidelity = math.nan
    else:
        overlap = np.vdot(exact / exact_norm, midpoint / norm)
        fidelity = float(abs(overlap) ** 2)
    return FidelityMetrics(
        amplitudes=len(exact),
        contained_amplitudes=contained,
        containment_rate=contained / len(exact),
        midpoint_fidelity=fidelity,
        midpoint_l2_error=float(np.linalg.norm(midpoint - exact)),
        midpoint_norm=norm,
        mean_diameter=float(np.mean(diameters)),
        max_diameter=float(np.max(diameters)),
        l2_radius=float(np.linalg.norm(radii)),
        certified_l2_error_bound=certified_l2_error_bound,
    )
