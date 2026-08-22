"""Generate paper-ready plots from checked-in raw benchmark results."""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict
from pathlib import Path
import statistics

import matplotlib.pyplot as plt


COLORS = {
    "Aer": "#4477AA",
    "MQT DDSIM": "#228833",
    "Coto exact": "#CC6677",
    "Coto B=4": "#AA3377",
}


def read_rows(path: Path) -> list[dict[str, str]]:
    with path.open(encoding="utf-8") as stream:
        return list(csv.DictReader(stream))


def engine_label(row: dict[str, str]) -> str | None:
    if row["engine"] == "aer":
        return "Aer"
    if row["engine"] == "mqt_ddsim":
        return "MQT DDSIM"
    if row["reduction_max_nodes"] == "exact":
        return "Coto exact"
    if row["reduction_max_nodes"] == "4":
        return "Coto B=4"
    return None


def plot_scaling(rows: list[dict[str, str]], output: Path) -> None:
    selected = [row for row in rows if row["circuit"].startswith("contrived/separable_n")]
    grouped: dict[tuple[str, int], list[dict[str, str]]] = defaultdict(list)
    for row in selected:
        label = engine_label(row)
        if label:
            grouped[label, int(row["qubits"])].append(row)

    figure, axes = plt.subplots(1, 2, figsize=(7.1, 2.8))
    for label in COLORS:
        points: list[tuple[int, float, float]] = []
        for (candidate, qubits), samples in grouped.items():
            successful = [sample for sample in samples if sample["status"] == "ok"]
            if candidate == label and successful:
                points.append(
                    (
                        qubits,
                        statistics.median(
                            float(sample["process_wall_seconds"]) for sample in successful
                        ),
                        statistics.median(float(sample["peak_rss_bytes"]) for sample in successful),
                    )
                )
        if points:
            points.sort()
            axes[0].plot(
                [point[0] for point in points],
                [point[1] for point in points],
                "o-",
                label=label,
                color=COLORS[label],
            )
            axes[1].plot(
                [point[0] for point in points],
                [point[2] / 2**20 for point in points],
                "o-",
                label=label,
                color=COLORS[label],
            )
    axes[0].set(ylabel="Wall time (s)", xlabel="Qubits", yscale="log")
    axes[1].set(ylabel="Peak RSS (MiB)", xlabel="Qubits", yscale="log")
    for axis in axes:
        axis.grid(alpha=0.25)
    axes[0].legend(fontsize=7)
    figure.tight_layout()
    figure.savefig(output, bbox_inches="tight")
    plt.close(figure)


def plot_tradeoff(
    benchmarks: list[dict[str, str]],
    fidelity: list[dict[str, str]],
    output: Path,
) -> None:
    memory: dict[tuple[str, str], list[float]] = defaultdict(list)
    for row in benchmarks:
        if row["engine"] == "coto" and row["status"] == "ok":
            memory[row["circuit"], row["reduction_max_nodes"]].append(
                float(row["coto_structural_bytes"])
            )
    figure, axis = plt.subplots(figsize=(3.5, 2.8))
    for circuit in sorted({row["circuit"] for row in fidelity}):
        points: list[tuple[float, float]] = []
        for row in fidelity:
            key = (circuit, row["reduction_max_nodes"])
            value = float(row["l2_radius"])
            if row["circuit"] == circuit and key in memory and value > 0:
                points.append((statistics.median(memory[key]) / 1024, value))
        if points:
            points.sort()
            axis.plot(
                [point[0] for point in points],
                [point[1] for point in points],
                "o-",
                label=circuit.split("/")[-1],
                markersize=3,
            )
    axis.set(
        xlabel="Coto structural memory (KiB)",
        ylabel=r"$\ell_2$ enclosure radius",
        xscale="log",
        yscale="log",
    )
    axis.grid(alpha=0.25)
    axis.legend(fontsize=5, ncol=2)
    figure.tight_layout()
    figure.savefig(output, bbox_inches="tight")
    plt.close(figure)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--benchmarks", type=Path, default=Path("results/benchmarks.csv"))
    parser.add_argument("--fidelity", type=Path, default=Path("results/fidelity.csv"))
    parser.add_argument("--output-dir", type=Path, default=Path("results/figures"))
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    benchmark_rows = read_rows(args.benchmarks)
    plot_scaling(benchmark_rows, args.output_dir / "scaling.pdf")
    plot_tradeoff(benchmark_rows, read_rows(args.fidelity), args.output_dir / "tradeoff.pdf")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
