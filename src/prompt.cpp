#include <iostream>
#include <functional>
#include <qasm.h>
#include <qasm/error.h>

/// @brief Main loop for the interpreter (interative mode)
void interpreter_main_loop();

void print_prompt();
void print_result(const std::string &result);

/// @brief Process a line of input
/// @param line The line to process
/// @return true if we should quit the program, else false
bool process_line(const std::string &line);

bool is_quit_line(const std::string &line);

/// @brief Try to execute a function and catch any exceptions, returning a default value if an exception occurs
template <typename T>
T try_catch_qasm(const std::function<T()> &func, const T &default_value);

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        return try_catch_qasm<int>(
            [&]()
            {
                qasm::fexec(argv[1]);
                return 0;
            },
            1);
    }
    else
    {
        interpreter_main_loop();
    }
    return 0;
}

void interpreter_main_loop()
{
    std::cout << "Coto QASM Interpreter" << std::endl;
    print_prompt();
    for (std::string line; std::getline(std::cin, line);)
    {
        if (process_line(line))
        {
            break;
        }
        print_prompt();
    }
}

void print_prompt()
{
    std::cout << "| " << std::flush;
}

void print_result(const std::string &result)
{
    if (result != "")
    {
        std::cout << "- " << result << std::endl;
    }
}

template <typename T>
T try_catch_qasm(const std::function<T()> &func, const T &default_value)
{
    try
    {
        return func();
    }
    catch (const SyntaxError &e)
    {
        std::cerr << "Syntax error: " << e.what() << '\n';
        return default_value;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Execution error: " << e.what() << '\n';
        return default_value;
    }
}

bool process_line(const std::string &line)
{
    static qasm::Runtime runtime = try_catch_qasm<qasm::Runtime>(
        [&]()
        {
            return qasm::exec(line);
        },
        qasm::exec(""));

    static bool is_first_line = true;
    if (is_first_line)
    {
        is_first_line = false;
        return false;
    }

    if (is_quit_line(line))
    {
        return true;
    }
    if (line.ends_with(';'))
    {
        runtime = try_catch_qasm<qasm::Runtime>(
            [&]()
            { return runtime.exec(line); },
            runtime);
        return false;
    }
    print_result(
        try_catch_qasm<std::string>(
            [&]()
            { return runtime.eval(line); },
            ""));
    return false;
}

bool is_quit_line(const std::string &line)
{
    const std::string cleaned = line.substr(0, line.find(";"));
    return cleaned == "quit" || cleaned == "exit";
}
