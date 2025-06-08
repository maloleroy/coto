#include <qasm/execute.h>
#include <qasm/statements.h>

static void execute(const std::vector<std::unique_ptr<Statement>> &statements, QasmContext& context);

void execute(const std::string &content, QasmContext& context)
{
    execute(parse_statements(content), context);
}

void execute(std::istream &stream, QasmContext& context)
{
    execute(parse_statements(stream), context);
}

static void execute(const std::vector<std::unique_ptr<Statement>> &statements, QasmContext& context)
{
    for (const auto &s : statements)
    {
        s->execute(context);
    }
}
