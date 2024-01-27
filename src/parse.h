// SPDX-License-Identifier: MIT License
#ifndef BF_CC_PARSE_H
#define BF_CC_PARSE_H 1

#include <variant>

#include "error.h"
#include "instr.h"

std::variant<char *, Err> read_content(const char *);

std::variant<OperationStream, Err> parse(const char *);

#endif /* BF_CC_PARSE_H */
