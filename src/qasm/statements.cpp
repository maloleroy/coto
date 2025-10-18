#include <iostream>
#include <algorithm>
#include <optional>
#include <sstream>
#include <cctype>

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
          name(content.substr(content.find(' ') + 1)),
          array_size(0)
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

    void execute(QasmContext &context) const override
    {
        context.storage.define_var(type_name, name, false);
    }

    const string type_name;
    const string name;
    size_t array_size;
};

class ArrayDefinitionStatement : public Statement
{
public:
    static bool is(const string &content)
    {
        // Check if content contains brackets, indicating an array definition
        // Array definitions have pattern like: "qubits[5] q" or "qubit[n] varname"
        // NOT "x q[0]" or "cx q[0], q[1]" (gate applications)

        size_t bracket_start = content.find('[');
        if (bracket_start == string::npos || content.find(']') == string::npos)
            return false;

        // For a valid array definition, the type name (before bracket) should be
        // at the beginning of the string and followed immediately by bracket
        // Also, there should be a space between the closing bracket and the variable name

        // Get the part before the bracket
        string before_bracket = content.substr(0, bracket_start);

        // Check if this looks like a type name (no spaces, valid identifier)
        if (before_bracket.empty() || before_bracket.find(' ') != string::npos)
            return false;

        if (!is_valid_identifier(before_bracket))
            return false;

        // Check if there's a space after the closing bracket
        size_t bracket_end = content.find(']');
        if (bracket_end + 1 >= content.length())
            return false;

        // Must have a space after the bracket
        if (content[bracket_end + 1] != ' ' && !std::isspace(static_cast<unsigned char>(content[bracket_end + 1])))
            return false;

        return true;
    }

    ArrayDefinitionStatement(string content)
    {
        // Parse type name, array name, and size
        // Expected format: "qubits[5] q" or "qubit[n] varname"

        size_t bracket_start = content.find('[');
        size_t bracket_end = content.find(']');

        if (bracket_start == string::npos || bracket_end == string::npos || bracket_end <= bracket_start)
        {
            throw SyntaxError("Invalid array syntax in definition statement");
        }

        // Extract type name
        type_name = content.substr(0, bracket_start);
        if (type_name.empty() || !is_valid_identifier(type_name))
        {
            throw SyntaxError("Invalid type name identifier in array definition statement");
        }

        // Extract array size
        string size_str = content.substr(bracket_start + 1, bracket_end - bracket_start - 1);
        if (size_str.empty())
        {
            throw SyntaxError("Array size cannot be empty");
        }

        try
        {
            array_size = std::stoul(size_str);
        }
        catch (...)
        {
            throw SyntaxError("Invalid array size: " + size_str);
        }

        if (array_size == 0)
        {
            throw SyntaxError("Array size must be greater than 0");
        }

        // Extract variable name
        string after_bracket = content.substr(bracket_end + 1);
        // Trim leading whitespace
        size_t name_start = after_bracket.find_first_not_of(" \t");
        if (name_start == string::npos)
        {
            throw SyntaxError("Variable name missing in array definition statement");
        }

        name = after_bracket.substr(name_start);
        if (!is_valid_identifier(name))
        {
            throw SyntaxError("Invalid identifier in array definition statement");
        }
    }

    void execute(QasmContext &context) const override
    {
        context.storage.define_var_array(type_name, name, array_size, false);
    }

    string type_name;
    string name;
    size_t array_size;
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

    void execute(QasmContext &context) const override
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
        // Check if there are parentheses (indicating parameters)
        size_t paren_start = content.find('(');

        if (paren_start == string::npos)
        {
            // No parameters - simple gate name followed by space and qubits
            size_t gate_end = content.find(' ');
            if (gate_end == string::npos)
                return false;

            string gate_name = content.substr(0, gate_end);
            return Gate::exists(gate_name);
        }
        else
        {
            // Gate has parameters, find the closing parenthesis
            size_t paren_end = content.find(')', paren_start);
            if (paren_end == string::npos)
                return false;

            // Check what comes after the closing paren
            size_t after_paren = paren_end + 1;
            while (after_paren < content.size() && std::isspace(static_cast<unsigned char>(content[after_paren])))
                after_paren++;

            // If nothing after paren (e.g., gphase(pi/2)), extract full gate name with params
            if (after_paren >= content.size())
            {
                string gate_name = content.substr(0, paren_end + 1);
                return Gate::exists(gate_name);
            }

            // Otherwise, find the space separating gate from qubits
            // The gate name is everything from start to paren_end (inclusive)
            // Then there might be a space, then qubits
            size_t space_pos = content.find(' ', paren_end);
            if (space_pos == string::npos)
                return false;

            string gate_name = content.substr(0, space_pos);
            return Gate::exists(gate_name);
        }
    }

    GateApplyStatement(const string &content)
    {
        // Find the end of the gate name (including parameters if present)
        size_t paren_start = content.find('(');
        size_t gate_end;

        if (paren_start == string::npos)
        {
            // No parameters - simple gate name
            gate_end = content.find(' ');
            if (gate_end == string::npos)
                gate_end = content.size();
        }
        else
        {
            // Gate has parameters, find the closing parenthesis
            size_t paren_end = content.find(')', paren_start);
            gate_end = paren_end + 1;

            // Check if there's a space after the closing paren (indicating qubits follow)
            while (gate_end < content.size() && std::isspace(static_cast<unsigned char>(content[gate_end])))
                gate_end++;

            // If we're at the end, this is a 0-qubit gate (like gphase)
            if (gate_end >= content.size())
            {
                gateName = content.substr(0, paren_end + 1);
                return;
            }

            // Find the space that separates gate from qubits
            gate_end = content.find(' ', paren_end);
            if (gate_end == string::npos)
                gate_end = content.size();
        }

        auto qubits_str = content.substr(gate_end + 1);
        gateName = content.substr(0, gate_end);
        // Trim whitespace from gateName
        while (!gateName.empty() && std::isspace(static_cast<unsigned char>(gateName.back())))
            gateName.pop_back();

        qubits_names.reserve(std::ranges::count(qubits_str, ',') + 1);

        while (qubits_str.size() > 0)
        {
            // Skip leading whitespace
            while (qubits_str.size() > 0 && std::isspace(static_cast<unsigned char>(qubits_str[0])))
            {
                qubits_str = qubits_str.substr(1);
            }
            if (qubits_str.empty())
                break;

            auto comma_pos = qubits_str.find(',');
            if (comma_pos == string::npos)
            {
                // Last qubit - trim trailing whitespace
                auto trimmed = qubits_str;
                while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back())))
                {
                    trimmed.pop_back();
                }
                if (!trimmed.empty())
                {
                    qubits_names.push_back(trimmed);
                }
                break;
            }
            auto qubit = qubits_str.substr(0, comma_pos);
            // Trim trailing whitespace from qubit
            while (!qubit.empty() && std::isspace(static_cast<unsigned char>(qubit.back())))
            {
                qubit.pop_back();
            }
            if (!qubit.empty())
            {
                qubits_names.push_back(qubit);
            }
            qubits_str = qubits_str.substr(comma_pos + 1);
        }
    }

    void execute(QasmContext &context) const override
    {
        context.apply_gate(Gate::from_name(gateName), qubits_names);
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

    void execute(QasmContext &context) const override {};

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

    void execute(QasmContext &context) const override {};

private:
    const string file_path;
};

class ForBeginStatement : public Statement
{
public:
    ForBeginStatement(const string &content) : content(content) {};

    void execute(QasmContext &context) const override {};

    const string content;
};

class ForEndStatement : public Statement
{
public:
    ForEndStatement(const string &content) : content(content) {};

    void execute(QasmContext &context) const override {};

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

    void execute(QasmContext &context) const override
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
        else if (content == "@memory" || content == "@mem")
        {
            context.print_diagram_memory_usage();
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
        if (ArrayDefinitionStatement::is(ss.content))
        {
            return std::make_unique<ArrayDefinitionStatement>(ss.content);
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
