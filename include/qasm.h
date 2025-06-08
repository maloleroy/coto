/**
 * @file qasm.h
 * @brief General header for the QASM library
 */
#pragma once

#include <istream>
#include <qasm/context.h> // Needed for QasmContext

/// @brief Handles everything related to QASM parsing and execution
namespace qasm
{
    class Runtime
    {
    public:
        Runtime();

        // Move constructor and move assignment operator
        Runtime(Runtime&& other) noexcept;
        Runtime& operator=(Runtime&& other) noexcept;

        // Delete copy constructor and copy assignment operator
        Runtime(const Runtime&) = delete;
        Runtime& operator=(const Runtime&) = delete;

        Runtime&& exec(const std::string &content);
        Runtime&& exec(std::istream &stream);
        Runtime&& fexec(const std::string &file_path);
        std::string eval(const std::string &identifier) const;

    private:
        QasmContext context; // Direct member
    };

    Runtime exec(const std::string &content);

    Runtime exec(std::istream &stream);

    Runtime fexec(const std::string &file_path);

    std::string eval(const std::string &identifier);
}
