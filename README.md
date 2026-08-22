# Coto - Abstract Additive Quantum Decision Diagrams

[![GitHub](https://img.shields.io/badge/GitHub-%23121011.svg?logo=github&logoColor=white)](https://github.com/maloleroy/coto)
[![GitHub Actions](https://img.shields.io/badge/GitHub_Actions-2088FF?logo=github-actions&logoColor=white)](https://github.com/maloleroy/coto/actions)
[![Build Status (GitHub Actions)](https://github.com/maloleroy/coto/workflows/CMake%20Tests/badge.svg)](https://github.com/maloleroy/coto/actions?query=workflow%3A"CMake%20Tests")
[![C++](https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white)](https://en.cppreference.com/w/cpp/23)
<a href="https://gitmoji.dev">
  <img
    src="https://img.shields.io/badge/gitmoji-%20😜%20😍-FFDD67.svg?style=flat-square"
    alt="Gitmoji"
  />
</a>
[![PlantUML](https://img.shields.io/badge/PlantUML-FABD14?logo=uml&logoColor=000)](http://www.plantuml.com/)

Theoretical research, experiments and implementation of _abstract additive quantum decision diagrams_, as part of a student research project.

## Scientific manuscript and artifacts

The author-facing ACM manuscript, reproducibility commands, and known-limitations report are
in [`paper/`](paper/). Checked-in raw benchmark/fidelity data and paper figures are under
[`analysis/results/`](analysis/results/); the longer-form theoretical notes remain in [`doc/`](doc/).

## Building

To build the project, you must have Clang, CMake and Ninja installed. The code uses some C++23 features, so a recent version of `clang++` is required. *Other versions probably work*, but I personally use the following on my machine:
* Clang 19.1.7
* CMake 3.31.6
* Ninja 1.12.1

1. Configure the project
```bash
git submodule update --init --recursive
export CXX=$(which clang++)
cmake -S . -B build -GNinja
```
2. Build the project
```bash
cmake --build build
```
3. To run the tests (optional)
```bash
cd build
ctest
```

## QASM interactive interpreter

After building the project (steps 1. and 2. above), simply run `./build/prompt`. You can then enter QASM instructions, for example
```
qubit a;
qubit b;
h a;
p(pi/3) b;
cx a b;
h b;
```

This prompt utility comes with a few available **run statements** that should be `;`-terminated:
* `@build`, `@inst`, `@instantiate` - create a new diagram
* `@list`, `@actions` - list the actions (gate applicaitons) to be performed
* `@display`, `@evaluate`, `@eval` - display the evaluation of the current diagram
* `@intervals` - emit machine-readable certified interval amplitudes (limited to 16 qubits)
* `@describe`, `@desc` - display the description of the current diagram
* `@memory`, `@mem` - display the memory usage of the current diagram
* `@reduce(N)` - enable approximate simulation and cap each nonterminal level at `N` nodes after every gate
* `@exact` - disable automatic reduction (the default)
* `@help`, `@man`, `@manual` - display a help message

Applying gates to qubits is deferred until a statement needs the diagram. `@display`,
`@describe`, and `@memory` all simulate every queued gate first.

The `prompt` executable can also take a `.qasm` file path as an argument, in this case it quits after executing all statements in the file, without entering interactive mode.

## Repository architecture

Presentation of this repository's main directories:
-  `.github/workflows` GitHub actions continuous integration (CI) and continous deployment (CD) configuration files
    - Runs the unit tests that are in the `test` directory and marked as ready for testing in `CMakeLists.txt`
    - Generate the docs using Doxygen and publish them to [maloleroy.github.io/coto](https://maloleroy.github.io/coto/) using GitHub Pages
-  `archives` Legacy documents or code, that still should be indexed
-  `doc` Theory documents (basically, LaTeX)
-  `include` Code header files (`.h` C++ files)
-  `ref` References, code or case studies that can be used as examples
-  `s6` Final report and presentation for semester 6 (in French)
-  `s7` Final report and presentation for semester 6 (in French)
-  `src` Source code (`.cpp` C++ files, not headers)
-  `test` Test code (`.cpp` C++ files)
