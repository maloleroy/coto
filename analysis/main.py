def main():
    import qasm_bench
    import transpile
    import memory

    for id in qasm_bench.all_ids():
        print(f"Processing {id}...")
        qasm = qasm_bench.content(id)
        transpiled_qasm = transpile.transpile(qasm)
        try:
            mem = memory.memory_from_qasm(transpiled_qasm)
        except RuntimeError:
            with open(f"error_{id.replace('/', '_')}.qasm", "w") as f:
                f.write(transpiled_qasm)
            raise
        print(f"{id}: {mem}")


if __name__ == "__main__":
    main()
