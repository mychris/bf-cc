// SPDX-License-Identifier: MIT License
#include "parse.h"

#if defined(_WIN64)
#define IS_WINDOWS 1
#elif defined(__linux__)
#define IS_LINUX 1
#else
#error Unsupported plattform
#endif

#if defined(IS_WINDOWS)
#include <windows.h>
#endif
#if defined(IS_LINUX)
#include <fcntl.h>
#include <unistd.h>
#endif

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#include "error.h"
#include "instr.h"

std::variant<OperationStream, Err> Parse(const std::string_view input) {
  OperationStream stream = OperationStream::Create();
  std::vector<Operation *> jump_stack{};
  for (const char c : input) {
    switch (c) {
    case '+': {
      stream.Append(Instruction::INCR_CELL, 1, 0);
    } break;
    case '-': {
      stream.Append(Instruction::DECR_CELL, 1, 0);
    } break;
    case '>': {
      stream.Append(Instruction::INCR_PTR, 1);
    } break;
    case '<': {
      stream.Append(Instruction::DECR_PTR, 1);
    } break;
    case ',': {
      stream.Append(Instruction::READ, 0, 0);
    } break;
    case '.': {
      stream.Append(Instruction::WRITE, 0, 0);
    } break;
    case '[': {
      stream.Append(Instruction::JZ, 0);
      jump_stack.push_back(stream.Last());
      stream.Append(Instruction::LABEL, 0);
      jump_stack.push_back(stream.Last());
    } break;
    case ']': {
      Operation *label{jump_stack.back()};
      jump_stack.pop_back();
      Operation *other{jump_stack.back()};
      jump_stack.pop_back();
      stream.Append(Instruction::JNZ, (Operation::operand_type) label);
      label->SetOperand1((Operation::operand_type) stream.Last());
      stream.Append(Instruction::LABEL, (Operation::operand_type) other);
      other->SetOperand1((Operation::operand_type) stream.Last());
    } break;
    default:
      break;
    }
  }
  if (0 != jump_stack.size()) {
    return Err::UnmatchedJump();
  }
  return stream;
}
