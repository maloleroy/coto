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


def memory(qasm_file: str) -> int:
    """Returns the memory usage in bytes of the quantum circuit defined in the given QASM file."""
    import subprocess
    import re

    # We call find_prompt_executable to get the path to the prompt executable
    prompt_executable = find_prompt_executable()

    result = subprocess.run(
        [prompt_executable, qasm_file], capture_output=True, text=True
    )
    if result.returncode != 0:
        raise RuntimeError("Failed to get memory usage.")

    match = re.search(r"~ memory usage:\s+(\d+)\s+bytes", result.stdout)
    if match:
        return int(match.group(1))
    raise RuntimeError("Failed to parse memory usage.")
