#include <iostream>
#include <functional>
#include <cstdlib>
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
T try_catch_qasm(const std::function<T()> &func, T default_value, bool interactive = false); // Changed: T default_value (by value)

// Global flag to track if we're in interactive mode
bool g_interactive_mode = false;

int main(int argc, char *argv[])
{
    // Check for interactive flag
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "-i" || arg == "--interactive")
        {
            g_interactive_mode = true;
            break;
        }
    }

    if (argc > 1 && argv[1][0] != '-')
    {
        return try_catch_qasm<int>(
            [&]()
            {
                qasm::fexec(argv[1]);
                return 0;
            },
            1,
            g_interactive_mode);
    }
    else
    {
        interpreter_main_loop();
    }
    return 0;
}

void interpreter_main_loop()
{
    if (g_interactive_mode)
    {
        std::cout << "Coto QASM Interpreter" << std::endl;
    }
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
    if (g_interactive_mode)
    {
        std::cout << "| " << std::flush;
    }
}

void print_result(const std::string &result)
{
    if (result != "")
    {
        std::cout << "- " << result << std::endl;
    }
}

template <typename T>
T try_catch_qasm(const std::function<T()> &func, T default_value, bool interactive)
{
    try
    {
        return func();
    }
    catch (const SyntaxError &e)
    {
        std::cerr << "Syntax error: " << e.what() << '\n';
        if (!interactive)
        {
            std::exit(1);
        }
        return std::move(default_value); // Changed: std::move
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Execution error: " << e.what() << '\n';
        if (!interactive)
        {
            std::exit(1);
        }
        return std::move(default_value); // Changed: std::move
    }
}

// Overload for void return type, no default value needed
void try_catch_qasm(const std::function<void()> &func, bool interactive)
{
    try_catch_qasm<bool>(
        [&]() -> bool
        {
            func();
            return true;
        },
        false,
        interactive);
}

bool process_line(const std::string &line)
{
    static qasm::Runtime runtime = try_catch_qasm<qasm::Runtime>(
        [&]()
        {
            return qasm::exec(line);
        },
        qasm::exec(""),
        g_interactive_mode); // This is an rvalue, fine for by-value parameter

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
        try_catch_qasm(
            [&]()
            { runtime = runtime.exec(line); },
            g_interactive_mode);
        return false;
    }
    print_result(
        try_catch_qasm<std::string>(
            [&]()
            { return runtime.eval(line); },
            "",
            g_interactive_mode));
    return false;
}

bool is_quit_line(const std::string &line)
{
    const std::string cleaned = line.substr(0, line.find(";"));
    return cleaned == "quit" || cleaned == "exit";
}
