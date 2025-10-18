def main():
    from memory import memory

    mem_usage = memory("../data/test.qasm")
    print(f"Memory usage: {mem_usage} bytes")


if __name__ == "__main__":
    main()
