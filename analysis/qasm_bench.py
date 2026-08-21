from collections.abc import Iterator, Sequence
from pathlib import Path
import re


ANALYSIS_ROOT = Path(__file__).resolve().parent
QASMBENCH_ROOT = ANALYSIS_ROOT / "data" / "QASMBench"
VALID_SIZES = ("small", "medium", "large")


def qubit_count(id: str) -> int:
    """Extract the qubit count from the ID. E.g., for 'small/adder_n4', return 4."""
    name = short_name(id)
    matches = re.findall(r"_n(\d+)(?:_|$)", name, flags=re.IGNORECASE)
    if not matches:
        raise ValueError(f"Cannot extract qubit count from ID: {id}")
    return int(matches[-1])


def short_name(id: str) -> str:
    return id.split("/")[-1]


def file_path(id: str) -> Path:
    """If the ID 'small/adder_n4', the path is 'data/small/adder_n4/adder_n4.qasm'"""
    path = QASMBENCH_ROOT / id / f"{short_name(id)}.qasm"
    if path.is_file():
        return path
    raise FileNotFoundError(f"QASM file not found: {path}")


def content(id: str) -> str:
    path = file_path(id)
    return path.read_text(encoding="utf-8")


def all_ids(sizes: Sequence[str] | None = None) -> Iterator[str]:
    """Yield QASMBench identifiers in a stable order."""
    sizes = VALID_SIZES if sizes is None else sizes
    for size in sizes:
        if size not in VALID_SIZES:
            raise ValueError(f"Unknown QASMBench size: {size}")
        size_dir = QASMBENCH_ROOT / size
        if not size_dir.is_dir():
            raise FileNotFoundError(f"QASMBench directory not found: {size_dir}")
        for entry in sorted(size_dir.iterdir()):
            if entry.is_dir() and (entry / f"{entry.name}.qasm").is_file():
                yield f"{size}/{entry.name}"
