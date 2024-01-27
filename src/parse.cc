// SPDX-License-Identifier: MIT License

#include "parse.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "error.h"
#include "instr.h"

#define OP_INCR '+'
#define OP_DECR '-'
#define OP_NEXT '>'
#define OP_PREV '<'
#define OP_READ ','
#define OP_WRIT '.'
#define OP_JMPF '['
#define OP_JMPB ']'

std::variant<OperationStream, Err> parse(const char *input) {
  OperationStream stream = OperationStream::Create();
  std::vector<Operation *> jump_stack = {};
  while (*input) {
    switch (*input) {
    case OP_INCR: {
      stream.Append(Instruction::INCR_CELL, 1);
    } break;
    case OP_DECR: {
      stream.Append(Instruction::DECR_CELL, 1);
    } break;
    case OP_NEXT: {
      stream.Append(Instruction::INCR_PTR, 1);
    } break;
    case OP_PREV: {
      stream.Append(Instruction::DECR_PTR, 1);
    } break;
    case OP_READ: {
      stream.Append(Instruction::READ, 0);
    } break;
    case OP_WRIT: {
      stream.Append(Instruction::WRITE, 0);
    } break;
    case OP_JMPF: {
      stream.Append(Instruction::JUMP_ZERO, 0);
      Operation *this_op = stream.Last();
      jump_stack.push_back(this_op);
    } break;
    case OP_JMPB: {
      Operation *other = jump_stack.back();
      jump_stack.pop_back();
      stream.Append(Instruction::JUMP_NON_ZERO, (Operation::operand_type) other);
      Operation *this_op = stream.Last();
      other->SetOperand1((Operation::operand_type) this_op);
    } break;
    default:
      break;
    }
    ++input;
  }
  if (0 != jump_stack.size()) {
    return Err::UnmatchedJump();
  }
  return stream;
}

std::variant<char *, Err> read_content(const char *filename) {
  char *content = NULL;
  off_t bytes_read = 0;
  const int fp = open(filename, O_RDONLY);
  if (0 > fp) {
    return Err::IO(errno);
  }
  const off_t fsize = lseek(fp, 0, SEEK_END);
  if (0 > fsize) {
    close(fp);
    return Err::IO(errno);
  }
  if (0 > lseek(fp, 0, SEEK_SET)) {
    close(fp);
    return Err::IO(errno);
  }
  content = (char *) calloc((size_t) fsize + 1, sizeof(char));
  while (bytes_read < fsize) {
    const ssize_t r = read(fp, content + bytes_read, (size_t) (fsize - bytes_read));
    if (0 > r && errno != EAGAIN) {
      free(content);
      close(fp);
      return Err::IO(errno);
    }
    bytes_read += r;
  }
  close(fp);
  return content;
}
