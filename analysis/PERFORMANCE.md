# Pre-benchmark performance audit

This audit was completed before collecting publication measurements. Its purpose is to
remove implementation artifacts that would otherwise dominate comparisons, and to set
safe limits for later experiments. These numbers are diagnostic microbenchmarks, not paper
results.

## Method

The fixture starts in `|0...0>`, applies a Hadamard gate to every qubit, and asks only for
the final structural memory footprint. This is deliberately simple: the resulting product
state has a one-node-per-level additive diagram, so exponential growth indicates avoidable
copying rather than intrinsic state complexity. Runs used the release-like GCC build on the
same workstation, `/usr/bin/time`, an 8-second timeout per process, and no state-vector
evaluation.

The original gate path cloned the whole diagram before every matrix operation. Profiling by
bounded size showed the clone and repeated vector-based node deduplication to be the dominant
costs. Gate application now builds the transformed DAG functionally, shares unchanged
subgraphs, and replaces the root atomically. Node collection and footprint traversal use hash
sets, and parent links are unique.

| Qubits | Structural bytes before | Structural bytes after | Time after |
| ---: | ---: | ---: | ---: |
| 8 | 55,200 | 1,848 | <0.01 s |
| 12 | 884,640 | 2,712 | <0.01 s |
| 16 | 14,155,680 | 3,576 | <0.01 s |
| 20 | >8 s (timeout) | 4,440 | <0.01 s |
| 32 | not attempted after timeout | 7,032 | <0.01 s |
| 64 | infeasible under old trend | 13,944 | <0.01 s |
| 128 | infeasible under old trend | 27,768 | <0.01 s |

The test `GateAppliersTest.separable_hadamards_keep_linear_structure` makes the essential
property deterministic: 64 Hadamards must leave exactly one reachable node per nonterminal
level and use less than 32 KiB of measured structural storage. Correctness fixtures compare
small exact state vectors for gate placement and operand order.

## Safety limits for publication runs

- Every simulator/circuit process must have an explicit wall-clock timeout.
- Start with at most 10 qubits and 60 seconds per QASMBench circuit, then increase one size
  tier only after inspecting completion time and peak RSS.
- Record failures and timeouts as rows; never silently omit them.
- Flush results after every circuit so an interrupted run remains usable.
- Do not evaluate Coto's full state vector when measuring structural memory.
- Measure peak RSS separately from Coto's reachable-structure byte estimate.
- Stop a batch before aggregate RSS approaches 24 GiB or free disk approaches 10 GiB.
- Use fixed circuit inputs, simulator versions, build configuration, and seeds where a backend
  is stochastic.

Validation consists of the complete C++/Python CTest suite in the normal build and an
AddressSanitizer/UndefinedBehaviorSanitizer build. Leak detection is disabled for the full
legacy suite because several older fixtures do not release their root diagrams and the
process-wide terminal singleton is intentionally retained. Focused ownership regressions run
with leak detection enabled; invalid accesses remain covered by ASan throughout the suite.
