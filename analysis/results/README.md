# Comparative benchmark results

These are the raw results used to assess Coto against Qiskit Aer's state-vector simulator
and MQT DDSIM's decision-diagram simulator. The checked-in CSV contains every attempted run,
including timeouts and memory-limit failures. `benchmarks.metadata.json` records the software
versions, harness commit, resource limits, and platform.

## Protocol

- 20 circuits: separable and GHZ families from 10 to 60 qubits, deterministic four-layer
  entangling circuits from 6 to 12 qubits, and four QASMBench circuits.
- Three fresh-process repetitions per engine and configuration, for 360 rows total.
- A 10-second wall-time limit and 4 GiB address-space limit per process.
- Aer uses `AerSimulator(method="statevector")`; MQT uses its `qasm_simulator`; each executes
  one measured shot. Coto runs exactly and with per-level node budgets 1, 4, and 16.
- GNU `time` measures whole-process wall time and peak RSS. Aer and MQT additionally report
  simulation-only time; Coto additionally reports reachable decision-diagram storage.
- Plots use the median of successful repetitions. Failed runs are not converted to censored
  numeric values and do not appear as successful points.

## Main observations

All 60 MQT DDSIM runs succeeded. Aer succeeded in all 36 runs at 20 qubits or fewer and hit
the memory limit in all 24 scaled runs at 30 qubits or more. Of 240 Coto runs, 234 succeeded;
only exact Coto timed out, on all three repetitions of the 10- and 12-qubit layered circuits.
Every budgeted Coto run completed.

Representative medians are:

| Circuit and configuration | Wall time | Peak RSS | Coto structure |
| --- | ---: | ---: | ---: |
| Separable, 60 qubits, Aer | memory limit | — | — |
| Separable, 60 qubits, MQT DDSIM | 0.390 s | 138.6 MiB | — |
| Separable, 60 qubits, Coto exact | 0.0078 s | 4.52 MiB | 12.3 KiB |
| GHZ, 60 qubits, MQT DDSIM | 0.414 s | 136.5 MiB | — |
| GHZ, 60 qubits, Coto exact | 0.0080 s | 4.37 MiB | 19.7 KiB |
| Layered, 8 qubits, Coto exact | 0.252 s | 8.06 MiB | 3.00 MiB |
| Layered, 8 qubits, Coto budget 4 | 0.0066 s | 4.45 MiB | 1.86 KiB |
| Layered, 10 qubits, Coto exact | timeout | — | — |
| Layered, 10 qubits, Coto budget 4 | 0.0055 s | 4.52 MiB | 2.27 KiB |

The scaled separable and GHZ cases demonstrate that structural simulators can avoid the
state-vector memory wall, but MQT DDSIM also handles these circuits well. The layered cases
isolate Coto's intended contribution: a fixed node budget turns exact exponential growth
into a small, terminating abstract result. These cases are deliberately contrived and must
be discussed as such rather than treated as a representative workload distribution.

## Interpretation limits

Whole-process timings and RSS include Python interpreter and package startup for Aer and MQT,
whereas Coto is a native executable. They are useful end-to-end measurements but not pure
kernel comparisons; the CSV's simulation-only column helps distinguish the two for the Python
engines. One measured shot also makes this an evolution benchmark, not a sampling-throughput
benchmark.

Most importantly, the current sound post-merge enclosure is often very coarse. The companion
fidelity dataset maintains 100% amplitude containment, but many budgeted layered/QAOA rows
have a zero midpoint and therefore no finite normalized midpoint fidelity. The benchmark
establishes reachability and a memory bound, not yet a compelling memory-versus-fidelity
curve. Precision-preserving sound propagation is a required follow-up before using that curve
as the paper's central empirical claim.
