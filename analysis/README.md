# Analysis of abstract additive quantum decision diagrams

This project contains reproducible tools for measuring Coto on QASMBench circuits. The
current collector records the final structural footprint of Coto's diagram, the circuit
size, elapsed wall-clock time, and any timeout or compatibility failure in CSV format.

The reported `memory_bytes` includes each reachable diagram node once, the allocated
capacity of its branch vectors, and its parent-pointer capacity. It is not the process's
peak resident-set size or allocator overhead; later comparative experiments should report
those separately.

## Setup

Clone the repository and its QASMBench submodule, then build Coto:

```sh
git clone --recurse-submodules git@github.com:maloleroy/coto.git
cd coto
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

For an existing clone, initialize the dataset with
`git submodule update --init --recursive`.

## Collect memory footprints

From `analysis/`, run a deliberately bounded first pass:

```sh
uv run python main.py --prompt ../build/prompt \
  --size small --max-qubits 10 --timeout 60 \
  --output results/memory-small.csv
```

Use repeated `--size` options to select several QASMBench groups. `--limit` restricts the
number of circuits, and each circuit has its own `--timeout`. Rows are flushed immediately,
so completed observations remain available if a later circuit fails. The command returns a
nonzero status if any row failed.

To collect an approximate run, add `--reduction-max-nodes N`. Coto then applies its
minimum-imprecision merge heuristic after every gate until every nonterminal level has at
most `N` nodes. The CSV records the budget (or `exact`) in every row; use separate output
files for each budget.

The compatibility layer translates the supported QASM 2 subset without evaluating the
full state vector. Measurements, barriers, identities, and classical registers are removed.
Classically-controlled operations are rejected and recorded as failures rather than silently
changing circuit semantics.

## Validation

Python checks are included in the main CTest suite. They can also be run directly:

```sh
uv run python -m unittest discover -s tests -v
uv run ruff check .
uv run ruff format --check .
```
