// SPDX-License-Identifier: MIT License
#include "instr.h"

#include <cstdio>

void Operation::Dump() const {
  printf("%zu ", (uintptr_t) this);
  switch (m.code) {
  case Instruction::NOP: {
    putchar(' ');
  } break;
  case Instruction::INCR_CELL: {
    putchar('+');
  } break;
  case Instruction::IMUL_CELL: {
    putchar('*');
  } break;
  case Instruction::DECR_CELL: {
    putchar('-');
  } break;
  case Instruction::SET_CELL: {
    putchar('=');
  } break;
  case Instruction::INCR_PTR: {
    putchar('>');
  } break;
  case Instruction::DECR_PTR: {
    putchar('<');
  } break;
  case Instruction::READ: {
    putchar(',');
  } break;
  case Instruction::WRITE: {
    putchar('.');
  } break;
  case Instruction::JUMP_ZERO: {
    putchar('[');
  } break;
  case Instruction::JUMP_NON_ZERO: {
    putchar(']');
  } break;
  case Instruction::FIND_CELL_HIGH: {
    putchar(')');
  } break;
  case Instruction::FIND_CELL_LOW: {
    putchar('(');
  } break;
  default: {
    putchar('?');
  } break;
  }
  printf(" %zd %zd\n", Operand1(), Operand2());
}

void OperationStream::Dump() {
  for (auto *instr : *this) {
    instr->Dump();
  }
}

bool OperationStream::Iterator::LookingAt(const std::initializer_list<Instruction> pattern) {
  if (0 == pattern.size()) {
    return true;
  }
  auto counter = 0u;
  Operation *current = m.current;
  auto pattern_iter = pattern.begin();
  const auto pattern_end = pattern.end();
  while (current && pattern_iter != pattern_end && current->OpCode() == *pattern_iter) {
    current = current->m.next;
    ++pattern_iter;
    ++counter;
  }
  return counter == pattern.size();
}
