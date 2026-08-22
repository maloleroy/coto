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
| Separable, 60 qubits, MQT DDSIM | 0.411 s | 138.8 MiB | — |
| Separable, 60 qubits, Coto exact | 0.0085 s | 4.48 MiB | 12.3 KiB |
| GHZ, 60 qubits, MQT DDSIM | 0.381 s | 136.6 MiB | — |
| GHZ, 60 qubits, Coto exact | 0.0091 s | 4.44 MiB | 19.7 KiB |
| Layered, 8 qubits, Coto exact | 0.252 s | 8.15 MiB | 3.00 MiB |
| Layered, 8 qubits, Coto budget 16 | 0.0636 s | 6.18 MiB | 1.32 MiB |
| Layered, 10 qubits, Coto exact | timeout | — | — |
| Layered, 10 qubits, Coto budget 4 | 0.0064 s | 4.52 MiB | 2.27 KiB |
| Layered, 12 qubits, Coto budget 4 | 0.0078 s | 4.60 MiB | 2.80 KiB |

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
