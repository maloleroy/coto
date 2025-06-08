#include <qasm/error.h>
#include <qasm/variables.h>
#include <qasm/context.h>

#include <vector>
#include <set>
#include <algorithm>

static std::vector<Var<int>> int_vars;
static std::vector<Var<bit>> bit_bars;
static std::vector<Var<qubit>> qubit_vars;

enum VarType
{
    IntVar,
    BitVar,
    QubitVar,
    NotDefined
};

VarType varType(const varname &name)
{
    for (const auto &v : int_vars)
    {
        if (v.name == name)
            return IntVar;
    }
    for (const auto &v : bit_bars)
    {
        if (v.name == name)
            return BitVar;
    }
    for (const auto &v : qubit_vars)
    {
        if (v.name == name)
            return QubitVar;
    }
    return NotDefined;
}

template <typename T>
std::string to_string(const Var<T> variable)
{
    return variable.type_name + ": " + std::to_string(variable.value);
}

std::string VariableStorage::var_to_string(const varname &name) const
{
    for (const auto &v : int_vars)
    {
        if (v.name == name)
            return to_string(v);
    }
    for (const auto &v : bit_bars)
    {
        if (v.name == name)
            return to_string(v);
    }
    for (const auto &v : qubit_vars)
    {
        if (v.name == name)
            return to_string(v);
    }
    throw VariableError("No such variable: " + name);
}

bool VariableStorage::is_name_reserved(const varname &name) noexcept
{
    static const std::set<std::string> reserved_names{"x", "h", "cx", "s"};
    return reserved_names.contains(name);
}

qubit VariableStorage::get_qubit_count() const noexcept
{
    return qubit_count;
}

void VariableStorage::define_var(const std::string &type_name, const varname &name, bool is_const)
{
    if (is_name_reserved(name))
    {
        throw VariableError("Trying to define a variable with a reserved name: " + name);
    }
    if (type_name == "int")
    {
        int_vars.push_back(Var<int>{type_name, name, is_const, false});
    }
    else if (type_name == "bit")
    {
        bit_bars.push_back(Var<bit>{type_name, name, is_const, false});
    }
    else if (type_name == "qubit")
    {
        qubit_vars.push_back(Var<qubit>{type_name, name, true, true, get_qubit_count()});
        qubit_count++;
    }
    else
    {
        throw VariableError("Unsupported variable type in definition: '" + type_name + "'");
    }
}

template <typename T>
void set_value(const varname &name, std::vector<Var<T>> *vars, const T &value)
{
    for (auto &v : *vars)
    {
        if (v.name == name)
        {
            if (v.is_const)
            {
                throw VariableError("Trying to assign to const variable " + name);
            }
            v.value = value;
            return;
        }
    }
}

void VariableStorage::assign_var(const varname &name, const std::string &value)
{
    switch (varType(name))
    {
    case IntVar:
        set_value(name, &int_vars, std::stoi(value));
        break;
    case BitVar:
        set_value(name, &bit_bars, std::stoi(value) != 0);
        break;
    case QubitVar:
        throw VariableError("Trying to assign to qubit " + name);
    default:
        throw VariableError("Trying to assign undefined variable" + name);
    }
}

qubit VariableStorage::get_qubit(const varname &name)
{
    for (auto v : qubit_vars)
    {
        if (v.name == name)
        {
            return v.value;
        }
    }
    throw VariableError("Qubit not defined: " + name);
}