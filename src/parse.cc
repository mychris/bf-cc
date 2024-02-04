// SPDX-License-Identifier: MIT License
#include "parse.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

#include "error.h"
#include "instr.h"

std::variant<OperationStream, Err> parse(const std::string_view input) {
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

static void close_file(int *fp) {
  close(*fp);
}

std::variant<std::string, Err> read_content(const std::string_view filename) {
  const std::unique_ptr<int, void (*)(int *)> fp{new int(open(filename.data(), O_RDONLY)), close_file};
  if (0 > *fp) {
    return Err::IO(errno);
  }
  const off_t fsize{lseek(*fp, 0, SEEK_END)};
  if (0 > fsize) {
    return Err::IO(errno);
  }
  if (0 > lseek(*fp, 0, SEEK_SET)) {
    return Err::IO(errno);
  }
  std::string content(static_cast<std::string::size_type>(fsize + 1), '\0');
  off_t bytes_read{0};
  while (fsize > bytes_read) {
    const ssize_t r = read(*fp, content.data() + bytes_read, (size_t) (fsize - bytes_read));
    if (0 > r && errno != EAGAIN) {
      return Err::IO(errno);
    }
    bytes_read += r;
  }
  return content;
}
