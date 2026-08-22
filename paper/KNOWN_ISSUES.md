# Author review: known limitations and possible defects

This list separates demonstrated guarantees from engineering and empirical limitations. It
should be reviewed manually before submission.

1. **The budget is not monotone in memory, runtime, or fidelity.** The cap applies to nodes
   per nonterminal level, not total branches. A merge can introduce additive interval branches
   that increase later work. In the eight-qubit layered case, budget 1 is slower and larger
   than several looser budgets. This is surprising but covered by the raw data, not hidden.
2. **The useful nominal path has an eight-qubit cutoff.** Above eight qubits, an abstract
   simulation switches to a compact uniform enclosure to prevent interval-branch explosion.
   This preserves termination and soundness but gives a zero midpoint and no useful fidelity.
3. **The global certificate is intentionally loose.** The implemented proof uses
   `||psi-m||_2 <= 1 + ||m||_2`. It certifies containment but does not quantify a tight error;
   improving it is the most important research follow-up.
4. **Machine-readable amplitude output stops at 16 qubits.** Structural memory and timing can
   be measured beyond that point, but exhaustive interval serialization cannot.
5. **Circuit semantics are a unitary subset.** Mid-circuit measurement and classical control
   are unsupported. The analysis compatibility layer removes final measurements and rejects
   classically controlled operations. Results must not be generalized to dynamic circuits.
6. **Benchmark timings include unequal startup costs.** Aer and DDSIM run in Python processes;
   Coto is a native executable. Simulation-only time is available only for the Python engines.
   One measured shot tests evolution/reachability, not sampling throughput.
7. **Soundness testing is finite.** All 48 checked rows contain every Aer reference amplitude,
   and deterministic C++ property tests exercise reductions, but this is validation rather
   than a machine-checked proof of the entire implementation.
8. **The legacy theory document overstates the old coefficient-wise merge.** General additive
   parents can invalidate correlations; the implementation now uses a sound uniform enclosure.
   The manuscript follows the implementation, while parts of `doc/Reduction/aaqdd.tex` still
   need a full editorial rewrite.
9. **Open engineering bugs remain out of paper scope.** GitHub issues #31 (reported prompt-exit
   crash), #10 (random diagram generator), and UI/parser issues should be triaged separately.
   They do not affect the file-mode benchmark path, but #31 deserves manual reproduction.
