# AAQDD manuscript and reproducibility

`main.tex` is an author-facing ACM `sigconf` manuscript. It uses the checked-in raw CSV
files and figures under `analysis/results/`; the older `doc/` tree remains the extended
theory reference.

## Build the manuscript

On Nix/NixOS, the repository supplies the exact TeX environment:

```sh
cd paper
nix-shell --run 'make pdf'
```

With an existing TeX Live installation, install the `acmart` package and run `make pdf`.
The output is `paper/main.pdf`.

## Reproduce the results

Initialize the QASMBench submodule and install the locked Python environment with `uv`, then
configure a release build as described in the root README. From `paper/`:

```sh
make verify
make experiments
```

`verify` runs the C++ and Python checks. `experiments` additionally replaces the checked-in
CSV and figures. The comparative run executes 396 fresh processes and deliberately records
expected timeout/memory-limit rows, so its collector returns status 1; the Makefile accepts
that status but no other failure code. Each process is limited to 10 seconds and 4 GiB.

For archival reproduction, use implementation commit
`2bec25369fb9ff0793f6b62b1ceb7e25034e1cbe`. The CSV metadata records the exact platform
and dependency versions of the checked-in run. See `KNOWN_ISSUES.md` before interpreting
the plots or preparing a submission.
