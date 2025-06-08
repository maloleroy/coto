#include <iostream>
#include <algorithm>
#include <optional>
#include <sstream>

#include <qasm/statements.h>
#include <qasm/variables.h>
#include <qasm/gate.h>
#include <qasm/context.h>

bool is_valid_identifier(string identifier)
{
    bool is_first_char = true;
    for (char c : identifier)
    {
        if (c < 'A' || c > 'z')
        {
            if (c == '_')
            {
                continue;
            }
            if (!is_first_char && c > '0' && c < '9')
            {
                continue;
            }
            return false;
        }
        is_first_char = false;
    }
    return true;
}

class DefinitionStatement : public Statement
{
public:
    static bool is(const string &content)
    {
        return std::ranges::count(content, ' ') == 1;
    }

    DefinitionStatement(string content)
        : type_name(content.substr(0, content.find(' '))),
          name(content.substr(content.find(' ') + 1))
    {
        if (!is_valid_identifier(type_name))
        {
            throw SyntaxError("Invalid type name identifier in definition statement");
        }
        if (!is_valid_identifier(name))
        {
            throw SyntaxError("Invalid name identifier in definition statement");
        }
    }

    void execute(QasmContext& context) const override
    {
        context.storage.define_var(type_name, name, false);
    }

    const string type_name;
    const string name;
};

class AssignmentStatement : public Statement
{
public:
    static bool is(const string &content)
    {
        return std::ranges::count(content, '=') == 1;
    }

    AssignmentStatement(const string &content)
    {
        auto eqPos = content.find('=');
        auto beforeEqual = content.substr(0, eqPos);
        auto spacePos = beforeEqual.find(' ');
        if (spacePos != string::npos)
        {
            if (!is_valid_identifier(beforeEqual.substr(0, spacePos)))
            {
                throw SyntaxError("Invalid type identifier in assignment statement");
            }
            if (!is_valid_identifier(beforeEqual.substr(spacePos + 1)))
            {
                throw SyntaxError("Invalid identifier in assignment statement");
            }
            type_name = beforeEqual.substr(0, spacePos);
            name = beforeEqual.substr(spacePos + 1);
            value = content.substr(eqPos + 1);
            return;
        }
        if (!is_valid_identifier(beforeEqual))
        {
            throw SyntaxError("Invalid identifier in assignment statement");
        }
        name = beforeEqual;
        value = content.substr(eqPos + 1);
    }

    void execute(QasmContext& context) const override
    {
        if (type_name.has_value())
        {
            context.storage.define_var(type_name.value(), name);
        }
        context.storage.assign_var(name, value);
    };

    std::optional<string> type_name;
    string name;
    string value;
};

class GateApplyStatement : public Statement
{
public:
    GateApplyStatement(string gateName, std::vector<string> qubits_names) : gateName(gateName), qubits_names(qubits_names) {}

    static bool is(const string &content)
    {
        return std::ranges::count(content, ' ') >= 1 && Gate::exists(content.substr(0, content.find(' ')));
    }

    GateApplyStatement(const string &content)
    {
        auto first_space_pos = content.find(' ');
        auto qubits_str = content.substr(first_space_pos + 1);
        gateName = content.substr(0, first_space_pos);
        qubits_names.reserve(std::ranges::count(qubits_str, ' ') + 1);

        while (qubits_str.size() > 0)
        {
            auto comma_pos = qubits_str.find(' ');
            if (comma_pos == string::npos)
            {
                qubits_names.push_back(qubits_str);
                break;
            }
            qubits_names.push_back(qubits_str.substr(0, comma_pos));
            qubits_str = qubits_str.substr(comma_pos + 1);
        }
    }

    void execute(QasmContext& context) const override
    {
        auto gate = Gate::from_name(gateName);
        context.apply_gate(&gate, qubits_names);
    };

    string gateName;
    std::vector<string> qubits_names;
};

class VersionStatement : public Statement
{
public:
    VersionStatement(const string &content) : version(content.length() > 9 ? content.substr(9) : "")
    {
        if (version != "3" && version != "3.0")
        {
            throw VersionError("Unsupported version '" + version + "'");
        }
    }

    static bool is(const string &content)
    {
        return content.starts_with("OPENQASM ");
    }

    void execute(QasmContext& context) const override {};

    const string version;
};

class IncludeStatement : public Statement
{
public:
    IncludeStatement(const string &content) : file_path(content.substr(9, content.size() - 1)) {};

    static bool is(const string &content)
    {
        return content.starts_with("include \"");
    }

    void execute(QasmContext& context) const override {};

private:
    const string file_path;
};

class ForBeginStatement : public Statement
{
public:
    ForBeginStatement(const string &content) : content(content) {};

    void execute(QasmContext& context) const override {};

    const string content;
};

class ForEndStatement : public Statement
{
public:
    ForEndStatement(const string &content) : content(content) {};

    void execute(QasmContext& context) const override {};

    const string content;
};

class RunStatement : public Statement
{
public:
    RunStatement(const string &content) : content(content) {};

    static bool is(const string &content)
    {
        return content.starts_with("@") && is_valid_identifier(content.substr(1));
    }

    void execute(QasmContext& context) const override
    {
        if (content == "@build" || content == "@inst" || content == "@instantiate")
        {
            context.create_diagram();
        }
        else if (content == "@list" || content == "@actions")
        {
            context.print_list_of_actions();
        }
        else if (content == "@display" || content == "@evaluate" || content == "@eval")
        {
            context.print_evaluation(); // implicitely runs unexecuted actions
        }
        else if (content == "@describe" || content == "@desc")
        {
            context.print_diagram_description();
        }
        else if (content == "@help" || content == "@man" || content == "@manual")
        {
            context.print_run_statements_help();
        }
        else
        {
            throw SyntaxError("Invalid run statement");
        }
    };

    const string content;
};

std::unique_ptr<Statement>
Statement::parse(const struct StatementString &ss)
{
    if (ss.delimiter == ';')
    {
        if (RunStatement::is(ss.content))
        {
            return std::make_unique<RunStatement>(ss.content);
        }
        if (VersionStatement::is(ss.content))
        {
            return std::make_unique<VersionStatement>(ss.content);
        }
        if (IncludeStatement::is(ss.content))
        {
            return std::make_unique<IncludeStatement>(ss.content);
        }
        if (AssignmentStatement::is(ss.content))
        {
            return std::make_unique<AssignmentStatement>(ss.content);
        }
        if (GateApplyStatement::is(ss.content))
        {
            return std::make_unique<GateApplyStatement>(ss.content);
        }
        if (DefinitionStatement::is(ss.content))
        {
            return std::make_unique<DefinitionStatement>(ss.content);
        }
    }
    else if (ss.delimiter == '{')
    {
        return std::make_unique<ForBeginStatement>(ss.content);
    }
    else if (ss.delimiter == '}')
    {
        return std::make_unique<ForEndStatement>(ss.content);
    }
    throw SyntaxError("Invalid statement delimiter");
}

std::vector<std::unique_ptr<Statement>> parse_statements(const std::vector<StatementString> &statementStrings)
{
    std::vector<std::unique_ptr<Statement>> stmts;
    for (StatementString ss : statementStrings)
    {
        stmts.push_back(Statement::parse(ss));
    }
    return stmts;
}

std::vector<std::unique_ptr<Statement>> parse_statements(std::istream &stream)
{
    auto statementStrings = parse_statements_strings(stream);
    return parse_statements(statementStrings);
}

std::vector<std::unique_ptr<Statement>> parse_statements(const string &content)
{
    std::istringstream stream(content);
    return parse_statements(stream);
}
