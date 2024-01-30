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
  case Instruction::JZ: {
    putchar('[');
  } break;
  case Instruction::JNZ: {
    putchar(']');
  } break;
  case Instruction::LABEL: {
    putchar('#');
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

void OperationStream::Swap(Operation *left, Operation *right) {
  if (left == m.head) {
    m.head = right;
  } else if (right == m.head) {
    m.head = left;
  }
  if (left == m.tail) {
    m.tail = right;
  } else if (right == m.tail) {
    m.tail = left;
  }
  // Swapping Node1 and Node2
  Operation *temp;
  temp = left->m.next;
  left->m.next = right->m.next;
  right->m.next = temp;
  if (left->m.next != NULL) left->m.next->m.prev = left;
  if (right->m.next != NULL) right->m.next->m.prev = right;
  temp = left->m.prev;
  left->m.prev = right->m.prev;
  right->m.prev = temp;
  if (left->m.prev != NULL) left->m.prev->m.next = left;
  if (right->m.prev != NULL) right->m.prev->m.next = right;
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
