/**
 * @file execute.h
 * @brief Execution of QASM statements as strings
 */

#pragma once
#include <istream>
#include <qasm/context.h>

void execute(const std::string& content, QasmContext& context);

void execute(std::istream& stream, QasmContext& context);
