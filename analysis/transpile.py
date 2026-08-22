# The goal of this script is to transpile a QASMBench file into a format
# suitable for our analysis tools.
from typing import Callable

TranspilingStep = Callable[[str], str]


def openqasm_version(qasm: str) -> str:
    return qasm.replace("OPENQASM 2.0;", "OPENQASM 3.0;")


def remove_comments(qasm: str) -> str:
    """Remove comments from QASM code."""
    return "\n".join(line.split("//", 1)[0] for line in qasm.splitlines())


def remove_barriers(qasm: str) -> str:
    """Remove barrier operations from QASM code."""
    lines = qasm.splitlines()
    lines = [line for line in lines if not line.strip().startswith("barrier")]
    return "\n".join(lines)


def remove_identities(qasm: str) -> str:
    """Remove identity operations from QASM code."""
    lines = qasm.splitlines()
    import re

    lines = [line for line in lines if not re.match(r"\s*id(?:\s|$)", line)]
    return "\n".join(lines)


def remove_ifs(qasm: str) -> str:
    """Reject classically-controlled operations instead of changing semantics."""
    import re

    lines = qasm.splitlines()
    if any(re.match(r"\s*if\s*\(", line) for line in lines):
        raise ValueError("Classically-controlled QASM operations are unsupported")
    return qasm


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
        r"qubits[\2] \1;",
        qasm,
    )


def split_gate_applications_on_arrays(qasm: str) -> str:
    """Split gate applications on qubit arrays into individual gate applications.

    At this point we already replaced qreg with qubit, so we only need to handle qubit arrays.
    1. Find all the identifiers that are declared as qubit arrays.
    2. For each gate application on an array, for example `h q`, we need to split it into individual applications like `h q[0];`, `h q[1];`, etc.
    """
    import re

    lines = qasm.splitlines()
    qubit_arrays = {}

    # First pass: find all qubit array declarations
    for line in lines:
        match = re.match(r"qubits\s*\[\s*(\d+)\s*\]\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*;", line)
        if match:
            name = match.group(2)
            size = int(match.group(1))
            qubit_arrays[name] = size
    # Second pass: split gate applications on arrays
    new_lines = []
    for line in lines:
        match = re.match(
            r"([a-zA-Z_][a-zA-Z0-9_]*(?:\([^;]*\))?)\s+"
            r"([a-zA-Z_][a-zA-Z0-9_]*)\s*;",
            line,
        )
        if match:
            gate = match.group(1)
            target = match.group(2)
            if target in qubit_arrays:
                size = qubit_arrays[target]
                for i in range(size):
                    new_lines.append(f"{gate} {target}[{i}];")
                continue  # Skip adding the original line
        new_lines.append(line)

    return "\n".join(new_lines)


def add_memory(qasm: str) -> str:
    """Add memory directive to QASM code."""
    return qasm + "\n@memory;\n"


def transpile(qasm: str, reduction_max_nodes: int | None = None) -> str:
    steps: list[TranspilingStep] = [
        openqasm_version,
        remove_comments,
        remove_barriers,
        remove_identities,
        remove_ifs,
        remove_measurements,
        remove_classical_registers,
        rename_qreg_to_qubits,
        split_gate_applications_on_arrays,
    ]
    for step in steps:
        qasm = step(qasm)
    if reduction_max_nodes is not None:
        if reduction_max_nodes <= 0:
            raise ValueError("reduction_max_nodes must be positive")
        qasm += f"\n@reduce({reduction_max_nodes});\n"
    return add_memory(qasm)
