/**
 * @file read.h
 * @brief Read a QASM file and extract the statements
 */
#pragma once
#include <string>
#include <vector>
#include <fstream>

using std::string;

struct StatementString
{
    string content;
    char delimiter;
};

std::ifstream open_file(const string &file_path);

std::vector<struct StatementString> parse_statements_strings(std::istream &stream);

bool is_only_empty_characters(const string &str) noexcept;
