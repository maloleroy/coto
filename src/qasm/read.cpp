#include <numeric>
#include <qasm/read.h>
#include <qasm/error.h>
#include <istream>
#include <fstream>
#include <iostream>

std::vector<StatementString> statements_strings(const std::vector<std::string> &lines);
std::string before_in_line_comment(std::string line);
std::string before_block_comment(std::string line);
std::string after_block_comment(std::string line);
std::vector<std::string> read_file_lines_without_comments(std::istream &stream);

std::vector<StatementString> parse_statements_strings(std::istream &stream)
{
    auto lines = read_file_lines_without_comments(stream);
    return statements_strings(lines);
}

std::ifstream open_file(const std::string &file_path)
{
    std::ifstream file(file_path);

    if (!file.is_open())
    {
        throw FileError("Cannot open file");
    }

    return file;
}

std::vector<std::string> read_file_lines_without_comments(std::istream &stream)
{
    std::vector<std::string> lines;

    bool is_in_block_comment = false;
    for (std::string line; std::getline(stream, line);)
    {
        std::string cache;
        cache.reserve(line.size());
        if (!is_in_block_comment && line.find("/*") != std::string::npos)
        {
            is_in_block_comment = true;
            cache = before_block_comment(line);
            if (cache != "")
                lines.push_back(cache);
        }
        if (is_in_block_comment && line.find("*/") != std::string::npos)
        {
            is_in_block_comment = false;
            cache = after_block_comment(line);
            if (cache != "")
                lines.push_back(cache);
        }
        else if (!is_in_block_comment)
        {
            cache = before_in_line_comment(line);
            if (cache != "")
                lines.push_back(cache);
        }
    }
    return lines;
}

std::vector<StatementString> statements_strings(const std::vector<std::string> &lines)
{
    std::string content = std::accumulate(lines.begin(), lines.end(), std::string());
    std::vector<StatementString> statements;
    std::vector<char> delimiters;
    std::string current;

    for (char c : content)
    {
        if (c == ';' || c == '{' || c == '}')
        {
            statements.push_back({current, c});
            current = "";
        }
        else
        {
            current += c;
        }
    }

    return statements;
}

std::string before_in_line_comment(std::string line)
{
    return line.substr(0, line.find("//"));
}

std::string before_block_comment(std::string line)
{
    return line.substr(0, line.find("/*"));
}

std::string after_block_comment(std::string line)
{
    if (line.find("*/") + 2 >= line.size())
    {
        return "";
    }
    return line.substr(line.find("*/") + 2);
}

bool is_only_empty_characters(const std::string &str) noexcept
{
    return str.find_first_not_of(" \t\n\v\f\r;") == std::string::npos;
}
