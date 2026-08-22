"""Parse and evaluate Coto's machine-readable complex interval output."""

from __future__ import annotations

from dataclasses import dataclass
import math
import subprocess

from memory import find_prompt_executable


@dataclass(frozen=True)
class ComplexInterval:
    """A closed, axis-aligned rectangle in the complex plane."""

    re_min: float
    re_max: float
    im_min: float
    im_max: float

    def __post_init__(self) -> None:
        if not all(math.isfinite(value) for value in self.as_tuple()):
            raise ValueError("interval endpoints must be finite")
        if self.re_min > self.re_max or self.im_min > self.im_max:
            raise ValueError("interval endpoints are reversed")

    def as_tuple(self) -> tuple[float, float, float, float]:
        return self.re_min, self.re_max, self.im_min, self.im_max

    @property
    def midpoint(self) -> complex:
        return complex((self.re_min + self.re_max) / 2, (self.im_min + self.im_max) / 2)

    @property
    def radius(self) -> float:
        return math.hypot(self.re_max - self.re_min, self.im_max - self.im_min) / 2

    @property
    def diameter(self) -> float:
        return 2 * self.radius

    def contains(self, value: complex, tolerance: float = 1e-12) -> bool:
        return (
            self.re_min - tolerance <= value.real <= self.re_max + tolerance
            and self.im_min - tolerance <= value.imag <= self.im_max + tolerance
        )


@dataclass(frozen=True)
class CertifiedIntervalEvaluation:
    intervals: list[ComplexInterval]
    l2_error_bound: float


def parse_certified_interval_output(output: str) -> CertifiedIntervalEvaluation:
    """Strictly parse intervals and their optional global L2 certificate."""
    lines = [line.strip() for line in output.splitlines()]
    headers = [line for line in lines if line.startswith("~ intervals-v1 ")]
    if len(headers) != 1:
        raise RuntimeError(f"expected one intervals-v1 header, found {len(headers)}")
    try:
        count = int(headers[0].split()[2])
    except (IndexError, ValueError) as error:
        raise RuntimeError(f"invalid interval header: {headers[0]}") from error
    if count < 0:
        raise RuntimeError("negative interval count")

    parsed: dict[int, ComplexInterval] = {}
    for line in lines:
        if not line.startswith("~ interval "):
            continue
        fields = line.split()
        if len(fields) != 7:
            raise RuntimeError(f"invalid interval row: {line}")
        try:
            index = int(fields[2])
            interval = ComplexInterval(*(float(value) for value in fields[3:]))
        except (ValueError, TypeError) as error:
            raise RuntimeError(f"invalid interval row: {line}") from error
        if index in parsed:
            raise RuntimeError(f"duplicate interval index: {index}")
        parsed[index] = interval

    if "~ intervals-end" not in lines:
        raise RuntimeError("missing intervals-end marker")
    certificates = [line for line in lines if line.startswith("~ approximation-l2-error-bound ")]
    if len(certificates) > 1:
        raise RuntimeError("multiple approximation L2 certificates")
    try:
        l2_error_bound = float(certificates[0].split()[2]) if certificates else 0.0
    except (IndexError, ValueError) as error:
        raise RuntimeError("invalid approximation L2 certificate") from error
    if not math.isfinite(l2_error_bound) or l2_error_bound < 0:
        raise RuntimeError("approximation L2 certificate must be finite and nonnegative")
    expected = set(range(count))
    if set(parsed) != expected:
        raise RuntimeError(f"interval indices differ from 0..{count - 1}")
    return CertifiedIntervalEvaluation([parsed[index] for index in range(count)], l2_error_bound)


def parse_interval_output(output: str) -> list[ComplexInterval]:
    """Strictly parse an ``intervals-v1`` block from prompt output."""
    return parse_certified_interval_output(output).intervals


def certified_intervals_from_qasm(qasm: str, timeout: float = 60.0) -> CertifiedIntervalEvaluation:
    """Run Coto and return amplitude enclosures with their global certificate."""
    result = subprocess.run(
        [find_prompt_executable(), "-"],
        input=qasm,
        capture_output=True,
        text=True,
        timeout=timeout,
    )
    if result.returncode != 0:
        raise RuntimeError(f"Coto failed ({result.returncode}): {result.stderr.strip()}")
    return parse_certified_interval_output(result.stdout)


def intervals_from_qasm(qasm: str, timeout: float = 60.0) -> list[ComplexInterval]:
    """Run Coto and return its final complex amplitude enclosures."""
    return certified_intervals_from_qasm(qasm, timeout).intervals
