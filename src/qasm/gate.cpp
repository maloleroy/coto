#include <qasm/gate.h>
#include <qasm/error.h>
#include <qasm/context.h>

#include <algorithm>
#include <cctype>
#include <cmath>

// ... existing parse_phase_gate_phase and get_phase_gate_name functions ...

Gate::Gate(const std::string &name, const std::size_t size, const std::optional<int> &parameter, const std::vector<double> &float_params) noexcept
    : parameter(parameter),
      size(size),
      name(name),
      float_parameters(float_params)
{
}

std::string Gate::to_string() const noexcept
{
    return "gate: " + name + "[" + std::to_string(size) + "]";
}

static int parse_phase_gate_phase(const std::string &gateName)
{
    try
    {
        if (gateName.starts_with("p(2pi/"))
        {
            return std::stoi(gateName.substr(6, gateName.length() - 7));
        }
        return 2 * std::stoi(gateName.substr(5, gateName.length() - 6));
    }
    catch (std::invalid_argument &e)
    {
        throw VariableError("Phase gate phase is not a number");
    }
    catch (std::out_of_range &e)
    {
        throw VariableError("Phase gate phase out of range");
    }
}

static std::string get_phase_gate_name(int phase)
{
    if (phase == 1)
        return "p(0)";
    if (phase == 2)
        return "p(pi)";
    if (phase % 2 == 0)
        return "p(pi/" + std::to_string(phase / 2) + ")";
    return "p(2pi/" + std::to_string(phase) + ")";
}

[[nodiscard]]
bool is_phase_gate(const std::string &gateName)
{
    if (gateName == "p(0)" || gateName == "p(pi)" || gateName == "p(2pi)")
        return true;
    if (gateName.starts_with("p(2pi/"))
    {
        if (gateName.length() < 7 || gateName[gateName.length() - 1] != ')')
            return false;
        auto base = gateName[5] == '-' ? 7 : 6;
        return std::all_of(gateName.begin() + base, gateName.end() - 1, [](char c)
                           { return std::isdigit(static_cast<unsigned char>(c)); });
    }
    if (!gateName.starts_with("p(pi/") || gateName.length() < 6 || gateName[gateName.length() - 1] != ')')
        return false;
    auto base = gateName[5] == '-' ? 6 : 5;
    return std::all_of(gateName.begin() + base, gateName.end() - 1, [](char c)
                       { return std::isdigit(static_cast<unsigned char>(c)); });
}

bool Gate::exists(const std::string &gateName) noexcept
{
    // Check reserved (non-parameterized) gate names
    if (VariableStorage::is_name_reserved(gateName))
        return true;

    // Check phase gates
    if (is_phase_gate(gateName))
        return true;

    // Check parameterized gates
    if (gateName.starts_with("rx(") || gateName.starts_with("ry(") || gateName.starts_with("rz(") ||
        gateName.starts_with("cp(") || gateName.starts_with("crx(") || gateName.starts_with("cry(") ||
        gateName.starts_with("crz(") || gateName.starts_with("cu(") || gateName.starts_with("u(") ||
        gateName.starts_with("gphase("))
    {
        return gateName.back() == ')'; // Must end with ')'
    }

    return false;
}

// Helper to parse a single parameter expression (supporting negation and pi expressions)
static double parse_parameter_expression(const std::string &param_str)
{
    // Trim whitespace
    std::string param = param_str;
    param.erase(0, param.find_first_not_of(" \t"));
    param.erase(param.find_last_not_of(" \t") + 1);

    if (param.empty())
        throw std::invalid_argument("Empty parameter");

    // Check for negative sign
    bool is_negative = false;
    if (param[0] == '-')
    {
        is_negative = true;
        param = param.substr(1);
        // Trim whitespace after minus sign
        param.erase(0, param.find_first_not_of(" \t"));
    }

    double value = 0.0;

    // Check if it contains 'pi' (general pi expression handler)
    if (param.find("pi") != std::string::npos)
    {
        // Format can be: pi, N*pi, pi/D, N*pi/D
        // Examples: pi, 2*pi, pi/4, 3*pi/16, 2*pi/3

        size_t pi_pos = param.find("pi");

        // Parse the part before "pi"
        double multiplier = 1.0;
        if (pi_pos > 0)
        {
            std::string before_pi = param.substr(0, pi_pos);
            // Remove the '*' if present
            if (before_pi.back() == '*')
                before_pi.pop_back();
            if (!before_pi.empty())
            {
                multiplier = std::stod(before_pi);
            }
        }

        // Parse the part after "pi"
        double divisor = 1.0;
        if (pi_pos + 2 < param.length())
        {
            std::string after_pi = param.substr(pi_pos + 2);
            if (after_pi[0] == '/')
            {
                divisor = std::stod(after_pi.substr(1));
            }
            else
            {
                throw std::invalid_argument("Invalid pi expression format: " + param);
            }
        }

        value = (multiplier / divisor) * M_PI;
    }
    else
    {
        // Try to parse as a numeric literal
        value = std::stod(param);
    }

    return is_negative ? -value : value;
}

// Helper to parse rotation gate parameters like rx(0.5), ry(pi/4), etc.
static std::vector<double> parse_rotation_gate_params(const std::string &gateName, size_t param_count)
{
    // Format: gateName(param1, param2, ..., paramN)
    // Supports numeric literals and pi-related expressions, including negative values
    std::vector<double> params;

    size_t open_paren = gateName.find('(');
    size_t close_paren = gateName.rfind(')');

    if (open_paren == std::string::npos || close_paren == std::string::npos || close_paren <= open_paren)
    {
        throw VariableError("Invalid gate parameter format in " + gateName);
    }

    std::string params_str = gateName.substr(open_paren + 1, close_paren - open_paren - 1);

    try
    {
        // Handle simple case of single parameter (for rotation gates)
        if (param_count == 1)
        {
            params.push_back(parse_parameter_expression(params_str));
        }
        else
        {
            // For multiple parameters, split by comma
            size_t pos = 0;
            for (size_t i = 0; i < param_count; ++i)
            {
                size_t comma_pos = params_str.find(',', pos);
                std::string param = (comma_pos == std::string::npos)
                                        ? params_str.substr(pos)
                                        : params_str.substr(pos, comma_pos - pos);

                params.push_back(parse_parameter_expression(param));

                if (comma_pos == std::string::npos)
                    break;
                pos = comma_pos + 1;
            }
        }
    }
    catch (std::invalid_argument &e)
    {
        throw VariableError("Invalid gate parameter: " + params_str);
    }
    catch (std::out_of_range &e)
    {
        throw VariableError("Gate parameter out of range: " + params_str);
    }

    return params;
}

const Gate Gate::from_name(const std::string &gateName)
{
    if (is_phase_gate(gateName))
    {
        return Gate(gateName, 1, parse_phase_gate_phase(gateName));
    }
    // Single-qubit gates without parameters
    else if (gateName == "x" || gateName == "y" || gateName == "z" ||
             gateName == "h" || gateName == "s" || gateName == "sdg" ||
             gateName == "t" || gateName == "tdg" || gateName == "sx")
    {
        return Gate(gateName, 1);
    }
    // Single-qubit rotation gates with 1 parameter
    else if (gateName.starts_with("rx(") || gateName.starts_with("ry(") || gateName.starts_with("rz("))
    {
        auto params = parse_rotation_gate_params(gateName, 1);
        return Gate(gateName, 1, std::nullopt, params);
    }
    // Two-qubit gates without parameters
    else if (gateName == "swap" || gateName == "cx" || gateName == "cy" || gateName == "cz" ||
             gateName == "ch")
    {
        return Gate(gateName, 2);
    }
    // Controlled phase gate with 1 parameter
    else if (gateName.starts_with("cp("))
    {
        auto params = parse_rotation_gate_params(gateName, 1);
        return Gate(gateName, 2, std::nullopt, params);
    }
    // Controlled rotation gates with 1 parameter
    else if (gateName.starts_with("crx(") || gateName.starts_with("cry(") || gateName.starts_with("crz("))
    {
        auto params = parse_rotation_gate_params(gateName, 1);
        return Gate(gateName, 2, std::nullopt, params);
    }
    // Controlled U gate with 3 parameters
    else if (gateName.starts_with("cu("))
    {
        auto params = parse_rotation_gate_params(gateName, 3);
        return Gate(gateName, 2, std::nullopt, params);
    }
    // Three-qubit gates
    else if (gateName == "ccx" || gateName == "cswap")
    {
        return Gate(gateName, 3);
    }
    // Global phase gate
    else if (gateName.starts_with("gphase("))
    {
        auto params = parse_rotation_gate_params(gateName, 1);
        return Gate(gateName, 0, std::nullopt, params); // 0 qubits - affects global phase
    }
    // U gate (universal single-qubit gate with 3 parameters)
    else if (gateName.starts_with("u("))
    {
        auto params = parse_rotation_gate_params(gateName, 3);
        return Gate(gateName, 1, std::nullopt, params);
    }
    throw VariableError("Undefined gate " + gateName);
}

std::string gate_to_string_from_name(const std::string &name)
{
    if (Gate::exists(name))
    {
        return Gate::from_name(name).to_string();
    }
    throw VariableError("Undefined gate: " + name);
}
