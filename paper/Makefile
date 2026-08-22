PAPER = main

.PHONY: all pdf clean figures verify experiments

all: pdf

pdf:
	latexmk -pdf -interaction=nonstopmode -halt-on-error $(PAPER).tex

figures:
	cd ../analysis && uv run python plot_results.py

verify:
	cmake --build ../build-release -j2
	ctest --test-dir ../build-release --output-on-failure -j2
	cd ../analysis && COTO_PROMPT=../build-release/prompt uv run python -m unittest discover -s tests
	cd ../analysis && uv run ruff check .

experiments: verify
	cd ../analysis && COTO_PROMPT=../build-release/prompt uv run python collect_fidelity.py --budget 1 --budget 2 --budget 4 --budget 8 --budget 16 --timeout 60 --output results/fidelity.csv
	cd ../analysis && COTO_PROMPT=../build-release/prompt uv run python benchmarks.py --profile paper --repeats 3 --timeout 10 --memory-gib 4 --output results/benchmarks.csv || test $$? -eq 1
	$(MAKE) figures

clean:
	latexmk -C $(PAPER).tex
