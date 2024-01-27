#include "instr.h"

#include <cstdio>

void OperationStream::Dump() {
  auto iter = Begin();
  const auto end = End();
  while (iter != end) {
    Operation *code = *iter;
    printf("%zu ", (uintptr_t) code);
    switch (code->OpCode()) {
    case Instruction::ANY: {
      putchar('~');
    } break;
    case Instruction::NOP: {
      putchar(' ');
    } break;
    case Instruction::INCR_CELL: {
      putchar('+');
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
    }
    printf(" %zu %zu\n", code->Operand1(), code->Operand2());
    ++iter;
  }
}

void OperationStream::VisitPattern(std::initializer_list<Instruction> pattern,
                                     void (*fun)(OperationStream &, OperationStream::Iterator &)) {
  if (0 == pattern.size()) {
    return;
  }
  Instruction first = *pattern.begin();
  auto iter = Begin();
  const auto end = End();
  while (iter != end) {
    if ((*iter)->OpCode() == first) {
      auto stream_iter = From(*iter);
      auto stream_end = End();
      auto pattern_iter = pattern.begin();
      auto pattern_end = pattern.end();
      long matches = 0;
      while (stream_iter != stream_end && pattern_iter != pattern_end
             && (*pattern_iter == Instruction::ANY || *pattern_iter == (*stream_iter)->OpCode())) {
        ++stream_iter;
        ++pattern_iter;
        ++matches;
      }
      stream_iter = From(*iter);
      if (matches == (long) pattern.size()) {
        fun(*this, stream_iter);
      }
      stream_iter = From(*iter);
      while (matches > 0 && stream_iter != stream_end) {
        if ((*stream_iter)->OpCode() == Instruction::NOP) {
          Delete(stream_iter++);
        } else {
          ++stream_iter;
        }
        --matches;
      }
    }
    ++iter;
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
    current = current->Next();
    ++pattern_iter;
    ++counter;
  }
  return counter == pattern.size();
}
