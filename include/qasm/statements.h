/**
 * @file statements.h
 * @brief Parsing statements from QASM strings
 */
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <qasm/read.h>
#include <qasm/error.h>
#include <qasm/context.h>

class Statement {
public:
    static std::unique_ptr<Statement> parse(const struct StatementString &content);
    virtual ~Statement() = default;
    virtual void execute(QasmContext& context) const = 0;
};

std::vector<std::unique_ptr<Statement>> parse_statements(std::istream& stream);

std::vector<std::unique_ptr<Statement>> parse_statements(const string& content);
