# Analysis of abstract additive quantum decision diagrams

This project contains reproducible tools for measuring Coto on QASMBench circuits. The
current collector records the final structural footprint of Coto's diagram, the circuit
size, elapsed wall-clock time, and any timeout or compatibility failure in CSV format.

The reported `memory_bytes` includes each reachable diagram node once, the allocated
capacity of its branch vectors, and its parent-pointer capacity. It is not the process's
peak resident-set size or allocator overhead; the comparative collector below reports peak
resident-set size separately.

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
most `N` nodes. A general additive merge uses a compact uniform amplitude enclosure. After
the first merge, subsequent gates propagate the reduced diagram as a nominal approximation;
the unit-norm certificate described below provides the sound enclosure independently. The CSV
records the budget (or `exact`) in every row; use separate output files for each budget.

The compatibility layer translates the supported QASM 2 subset without evaluating the
full state vector. Measurements, barriers, identities, and classical registers are removed.
Classically-controlled operations are rejected and recorded as failures rather than silently
changing circuit semantics.

## Validate soundness and fidelity

The fidelity collector compares Coto's complex amplitude rectangles with a state vector
computed by Qiskit Aer. It reports the amplitude containment rate (the primary soundness
check), fidelity and L2 error of the rectangle midpoints, and aggregate rectangle radii.
It includes deterministic contrived circuits and a small QASMBench selection:

```sh
uv run python collect_fidelity.py --prompt ../build/prompt \
  --budget 1 --budget 2 --budget 4 --budget 8 --budget 16 \
  --timeout 60 --output results/fidelity.csv
```

The exact run and every approximate budget are written as individual rows and flushed
immediately. Package and platform versions are stored next to the CSV in
`fidelity.metadata.json`. Coto indexes qubit 0 as the most-significant state bit, while
Qiskit uses it as the least-significant bit; the collector explicitly reverses state indices
before comparing amplitudes. Mid-circuit measurements and classical control are rejected.

After a reduction, the reduced diagram is propagated as a nominal approximation. Machine-
readable intervals certify that midpoint independently: supported gates are unitary and the
initial state has unit norm, so the triangle inequality gives
`||psi - midpoint||_2 <= 1 + ||midpoint||_2`. Each complex rectangle is centered on the
nominal amplitude and widened outward by that global bound. This is intentionally
conservative, but it remains sound without repeatedly multiplying a global box by per-gate
row bounds, and it preserves a meaningful midpoint for fidelity measurement.

The nominal path is limited to eight qubits. Empirically, propagating interval-valued
branches beyond that point can itself become exponential on adversarial layered circuits.
For larger reduced simulations, Coto therefore propagates the compact sound uniform
enclosure directly. Certified interval output remains available through 16 qubits, but its
midpoint is zero in this safeguard regime. This cutoff preserves the reachability benefit
without presenting the nominal fidelity as a scalability guarantee.

## Compare simulators

The comparative collector runs Coto, Qiskit Aer, and MQT DDSIM in fresh processes. GNU
`time` records process peak RSS and wall time; Python workers also report simulation-only
time. Every run has an address-space limit and timeout, and failures remain in the CSV:

```sh
uv run python benchmarks.py --profile paper --repeats 3 \
  --timeout 10 --memory-gib 4 --prompt ../build/prompt \
  --output results/benchmarks.csv
uv run python plot_results.py
```

The paper profile includes scaled separable, GHZ, and deterministic entangling circuits plus
QASMBench instances. Aer uses its state-vector method and MQT uses its decision-diagram QASM
simulator; both execute one measured shot so circuit evolution cannot be optimized away. Coto
runs in exact mode and with node budgets 1, 4, and 16. Metadata records package versions,
the Git commit, platform, resource limits, and repetition count.

## Validation

Python checks are included in the main CTest suite. They can also be run directly:

```sh
uv run python -m unittest discover -s tests -v
uv run ruff check .
uv run ruff format --check .
```
