// SPDX-License-Identifier: MIT License
#ifndef BF_CC_PARSE_H
#define BF_CC_PARSE_H 1

#include <string_view>
#include <variant>

#include "error.h"
#include "instr.h"

std::variant<OperationStream, Err> Parse(const std::string_view);

#endif /* BF_CC_PARSE_H */
