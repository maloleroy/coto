# Comparative benchmark results

These are the raw results used to assess Coto against Qiskit Aer's state-vector simulator
and MQT DDSIM's decision-diagram simulator. The checked-in CSV contains every attempted run,
including timeouts and memory-limit failures. `benchmarks.metadata.json` records the software
versions, harness commit, resource limits, and platform.

## Protocol

- 22 circuits: separable and GHZ families from 10 to 60 qubits, deterministic four-layer
  entangling circuits from 6 to 12 qubits, 20-layer adversarial circuits at 20 and 30
  qubits, and four QASMBench circuits.
- Three fresh-process repetitions per engine and configuration, for 396 rows total.
- A 10-second wall-time limit and 4 GiB address-space limit per process.
- Aer uses `AerSimulator(method="statevector")`; MQT uses its `qasm_simulator`; each executes
  one measured shot. Coto runs exactly and with per-level node budgets 1, 4, and 16.
- GNU `time` measures whole-process wall time and peak RSS. Aer and MQT additionally report
  simulation-only time; Coto additionally reports reachable decision-diagram storage.
- Plots use the median of successful repetitions. Failed runs are not converted to censored
  numeric values and do not appear as successful points.

## Main observations

MQT DDSIM succeeded in 60 of 66 runs and timed out on every deep layered run. Aer succeeded
in 39 of 66 runs and hit the memory limit in 27 runs, including all repetitions of the deep
30-qubit case. Of 264 Coto runs, 252 succeeded; exact Coto timed out on all repetitions of
the 10- and 12-qubit layered circuits and both deep circuits. Every budgeted Coto run
completed.

Representative medians are:

| Circuit and configuration | Wall time | Peak RSS | Coto structure |
| --- | ---: | ---: | ---: |
| Separable, 60 qubits, Aer | memory limit | — | — |
| Separable, 60 qubits, MQT DDSIM | 0.422 s | 138.4 MiB | — |
| Separable, 60 qubits, Coto exact | 0.0086 s | 4.61 MiB | 12.3 KiB |
| GHZ, 60 qubits, MQT DDSIM | 0.401 s | 136.7 MiB | — |
| GHZ, 60 qubits, Coto exact | 0.0084 s | 4.38 MiB | 19.7 KiB |
| Layered, 8 qubits, Coto exact | 0.251 s | 8.21 MiB | 3.00 MiB |
| Layered, 8 qubits, Coto budget 16 | 0.0669 s | 6.21 MiB | 1.32 MiB |
| Layered, 10 qubits, Coto exact | timeout | — | — |
| Layered, 10 qubits, Coto budget 4 | 0.0070 s | 4.58 MiB | 2.27 KiB |
| Layered, 12 qubits, Coto budget 4 | 0.0059 s | 4.47 MiB | 2.80 KiB |
| Deep layered, 20 qubits, Aer | 0.652 s | 121.6 MiB | — |
| Deep layered, 20 qubits, MQT DDSIM | timeout | — | — |
| Deep layered, 20 qubits, Coto budget 4 | 0.0127 s | 4.71 MiB | 4.42 KiB |
| Deep layered, 30 qubits, Aer | memory limit | — | — |
| Deep layered, 30 qubits, MQT DDSIM | timeout | — | — |
| Deep layered, 30 qubits, Coto budget 4 | 0.0270 s | 4.80 MiB | 6.70 KiB |

The scaled separable and GHZ cases demonstrate that structural simulators can avoid the
state-vector memory wall, but MQT DDSIM also handles these circuits well. The layered cases
isolate Coto's intended contribution: a fixed node budget turns exact exponential growth
into a small, terminating abstract result. On the deep 30-qubit case, bounded Coto is the only
tested configuration to complete within both limits. These cases are deliberately contrived
and must be discussed as such rather than treated as a representative workload distribution.

## Interpretation limits

Whole-process timings and RSS include Python interpreter and package startup for Aer and MQT,
whereas Coto is a native executable. They are useful end-to-end measurements but not pure
kernel comparisons; the CSV's simulation-only column helps distinguish the two for the Python
engines. One measured shot also makes this an evolution benchmark, not a sampling-throughput
benchmark.

The companion fidelity dataset contains 48 exact and budgeted runs and maintains 100%
amplitude containment. On the eight-qubit layered circuit, budget 4 has midpoint fidelity
0.651 and certified L2 error bound 1.807; budget 16 reduces structural storage from 3.00 MiB
to 1.32 MiB. These quantities answer different questions: fidelity is an empirical comparison
with an exact reference, while the unit-norm triangle bound is a proof obligation and is
deliberately conservative.

Neither fidelity nor resource use is monotone in the node budget. Merging changes subsequent
sharing and interval propagation, so a smaller per-level cap can create more additive
branches; the layered eight-qubit budget-1 run is a concrete example. Above eight qubits the
implementation deliberately switches to a compact uniform enclosure. This makes the 10- and
12-qubit adversarial cases reachable in milliseconds, but their zero midpoint carries no
useful empirical fidelity. The results therefore support a tunable, sound memory--precision
mechanism and a reachability result, not a claim that every budget improves every workload.
