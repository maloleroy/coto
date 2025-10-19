def find_prompt_executable() -> str:
    """Finds the path to the prompt executable."""
    from os.path import isfile, join, dirname

    current_dir = dirname(__file__)
    possible_paths = [
        join(current_dir, "prompt"),
        join(current_dir, "build", "prompt"),
        join(current_dir, "..", "build", "prompt"),
        join(current_dir, "..", "..", "build", "prompt"),
    ]
    for path in possible_paths:
        if isfile(path):
            return path
    raise FileNotFoundError("Prompt executable not found.")


def memory_from_qasm(qasm: str) -> int:
    """Returns the memory usage in bytes of the quantum circuit defined in the given QASM file."""
    import subprocess

    # We call find_prompt_executable to get the path to the prompt executable
    prompt_executable = find_prompt_executable()

    # feed the QASM string via stdin (many tools accept "-" to mean read from stdin)
    result = subprocess.run(
        [prompt_executable, "-"], input=qasm, capture_output=True, text=True
    )
    if result.returncode != 0:
        raise RuntimeError("Failed to get memory usage: " + result.stderr)
    return _parse_memory_output(result.stdout)


def memory_from_path(qasm_path: str) -> int:
    """Returns the memory usage in bytes of the quantum circuit defined in the given QASM file path."""
    import subprocess

    # We call find_prompt_executable to get the path to the prompt executable
    prompt_executable = find_prompt_executable()

    result = subprocess.run(
        [prompt_executable, qasm_path], capture_output=True, text=True
    )
    if result.returncode != 0:
        raise RuntimeError("Failed to get memory usage: " + result.stderr)
    return _parse_memory_output(result.stdout)


def _parse_memory_output(output: str) -> int:
    import re

    match = re.search(r"~ memory usage:\s+(\d+)\s+bytes", output)
    if match:
        return int(match.group(1))
    raise RuntimeError(f"Failed to parse memory usage: {output}")
