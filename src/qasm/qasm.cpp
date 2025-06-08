#include <qasm.h>

#include <qasm/context.h> // Included via qasm.h, but good for clarity
#include <qasm/execute.h>
#include <qasm/variables.h>
#include <qasm/error.h>
#include <qasm/read.h>
#include <qasm/gate.h>

#include <iostream>
#include <filesystem>

using namespace qasm;

Runtime::Runtime()
{
}

// Move constructor
Runtime::Runtime(Runtime&& other) noexcept : context(std::move(other.context)) {}

// Move assignment operator
Runtime& Runtime::operator=(Runtime&& other) noexcept
{
    if (this != &other)
    {
        context = std::move(other.context);
    }
    return *this;
}

// Explicitly delete copy constructor and copy assignment operator
// Runtime::Runtime(const Runtime&) = delete; // These are in the header
// Runtime& Runtime::operator=(const Runtime&) = delete; // These are in the header

Runtime&& Runtime::exec(const std::string &content)
{
    execute(content, context); // Was impl->exec(content)
    return std::move(*this);
}

Runtime&& Runtime::exec(std::istream &stream)
{
    execute(stream, context); // Was impl->exec(stream)
    return std::move(*this);
}

Runtime&& Runtime::fexec(const std::string &file_path)
{
    // Logic from former RuntimeImpl::fexec
    if (!std::filesystem::exists(file_path))
    {
        std::string fp = file_path + ".qasm";
        if (std::filesystem::exists(fp))
        {
            auto f = open_file(fp);
            execute(f, context); // Call to global execute
            f.close();
            return std::move(*this);
        }
    }
    try
    {
        auto f = open_file(file_path);
        execute(f, context); // Call to global execute
        f.close();
    }
    catch (const FileError &e)
    {
        std::cerr << e.what() << '\n';
    }
    return std::move(*this);
}

std::string Runtime::eval(const std::string &identifier) const
{
    // Logic from former RuntimeImpl::eval
    if (is_only_empty_characters(identifier))
    {
        return "";
    }
    try
    {
        return gate_to_string_from_name(identifier);
    }
    catch (const VariableError &e)
    {
        return context.storage.var_to_string(identifier); // Direct use of context
    }
}

// Free functions remain the same as they operate on the Runtime public interface
Runtime qasm::exec(const std::string &content)
{
    Runtime rt;
    rt.exec(content);
    return rt;
}

Runtime qasm::exec(std::istream &stream) {
    Runtime rt;
    rt.exec(stream);
    return rt;
}

Runtime qasm::fexec(const std::string &file_path) {
    Runtime rt;
    rt.fexec(file_path);
    return rt;
}

std::string qasm::eval(const std::string &identifier) {
    return Runtime().eval(identifier);
}
