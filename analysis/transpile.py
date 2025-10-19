# The goal of this script is to transpile a QASMBench file into a format
# suitable for our analysis tools.
from typing import Callable

TranspilingStep = Callable[[str], str]


def openqasm_version(qasm: str) -> str:
    return qasm.replace("OPENQASM 2.0;", "OPENQASM 3.0;")


def remove_comments(qasm: str) -> str:
    """Remove comments from QASM code."""
    lines = qasm.splitlines()
    lines = [line for line in lines if not line.strip().startswith("//")]
    return "\n".join(lines)


def remove_barrier(qasm: str) -> str:
    """Remove barrier operations from QASM code."""
    lines = qasm.splitlines()
    lines = [line for line in lines if not line.strip().startswith("barrier")]
    return "\n".join(lines)


def remove_measurements(qasm: str) -> str:
    """Remove measurement operations from QASM code."""
    lines = qasm.splitlines()
    lines = [line for line in lines if not line.strip().startswith("measure")]
    return "\n".join(lines)


def remove_classical_registers(qasm: str) -> str:
    """Remove classical register operations from QASM code."""
    lines = qasm.splitlines()
    lines = [line for line in lines if not line.strip().startswith("creg")]
    return "\n".join(lines)


def rename_qreg_to_qubits(qasm: str) -> str:
    """Replace legacy qreg usage to QASM 3 qubit."""
    import re

    return re.sub(
        r"qreg\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\[\s*(\d+)\s*\]\s*;",
        r"qubit[\2] \1;",
        qasm,
    )


def add_eval(qasm: str) -> str:
    """Add eval directive to QASM code."""
    return qasm + "\n@eval;\n"


def add_memory(qasm: str) -> str:
    """Add memory directive to QASM code."""
    return qasm + "\n@memory;\n"


def transpile(qasm: str) -> str:
    steps: list[TranspilingStep] = [
        openqasm_version,
        remove_comments,
        remove_barrier,
        remove_measurements,
        remove_classical_registers,
        rename_qreg_to_qubits,
        add_eval,
        add_memory,
    ]
    for step in steps:
        qasm = step(qasm)
    return qasm
