from typing import Iterator


def short_name(id: str) -> str:
    return id.split("/")[-1]


def file_path(id: str) -> str:
    """If the ID 'small/adder_n4', the path is 'data/small/adder_n4/adder_n4.qasm'"""
    from os.path import isfile

    path = f"data/QASMBench/{id}/{short_name(id)}.qasm"
    if isfile(path):
        return path
    raise FileNotFoundError(f"QASM file not found: {path}")


def content(id: str) -> str:
    path = file_path(id)
    with open(path, "r") as f:
        return f.read()


def all_ids(sizes: list[str] = ["small", "medium", "large"]) -> Iterator[str]:
    from os import listdir
    from os.path import isdir

    for size in sizes:
        for i in listdir(f"data/QASMBench/{size}"):
            if isdir(f"data/QASMBench/{size}/{i}"):
                yield f"{size}/{i}"
