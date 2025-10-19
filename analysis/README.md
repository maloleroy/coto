# Analysis of abstract additive quantum decision diagrams

The goal of this project is to perform quantitative analysis of the performance of our implementation of abstract additive quantum decision diagrams.

This analysis focuses on three metrics:
- Memory usage
- Computing time
- Fidelity

## Implementation Details

- QASMBench is QASM 2, and we support a subset of QASM 3, so we have a in-house compatibility layer to translate between the two versions
