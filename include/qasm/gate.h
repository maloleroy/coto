/**
 * @file gate.h
 * @brief Handling QASM gates
 */
#pragma once
#include <string>
#include <optional>
#include <vector>

/// @brief A quantum gate
class Gate
{
public:
    [[nodiscard]]
    static bool exists(const std::string &gateName) noexcept;

    [[nodiscard]]
    static const Gate from_name(const std::string &gateName);

    std::string to_string() const noexcept;

    /// @brief The name of the gate
    const std::string name;

    /// @brief The number of qubits of the gate
    const size_t size;

    /// @brief Optional parameter for gates like phase gates
    const std::optional<int> parameter;

protected:
    Gate(const std::string &name, const std::size_t size, const std::optional<int> &parameter = std::nullopt) noexcept;
};

std::string gate_to_string_from_name(const std::string &name);
