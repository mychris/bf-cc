#include "instr.h"

#include <cstdio>

void Instr::Stream::Dump() {
  auto iter = Begin();
  const auto end = End();
  while (iter != end) {
    Instr *code = *iter;
    printf("%zu ", (uintptr_t) code);
    switch (code->OpCode()) {
    case InstrCode::ANY: {
      putchar('~');
    } break;
    case InstrCode::NOP: {
      putchar(' ');
    } break;
    case InstrCode::INCR_CELL: {
      putchar('+');
    } break;
    case InstrCode::DECR_CELL: {
      putchar('-');
    } break;
    case InstrCode::SET_CELL: {
      putchar('=');
    } break;
    case InstrCode::INCR_PTR: {
      putchar('>');
    } break;
    case InstrCode::DECR_PTR: {
      putchar('<');
    } break;
    case InstrCode::READ: {
      putchar(',');
    } break;
    case InstrCode::WRITE: {
      putchar('.');
    } break;
    case InstrCode::JUMP_ZERO: {
      putchar('[');
    } break;
    case InstrCode::JUMP_NON_ZERO: {
      putchar(']');
    } break;
    case InstrCode::FIND_CELL_HIGH: {
      putchar(')');
    } break;
    case InstrCode::FIND_CELL_LOW: {
      putchar('(');
    } break;
    }
    printf(" %zu %zu\n", code->Operand1(), code->Operand2());
    ++iter;
  }
}

void Instr::Stream::VisitPattern(std::initializer_list<InstrCode> pattern, void (*fun)(Instr *)) {
  if (0 == pattern.size()) {
    return;
  }
  InstrCode first = *pattern.begin();
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
             && (*pattern_iter == InstrCode::ANY || *pattern_iter == (*stream_iter)->OpCode())) {
        ++stream_iter;
        ++pattern_iter;
        ++matches;
      }
      if (matches == (long) pattern.size()) {
        fun(*iter);
      }
      stream_iter = From(*iter);
      while (matches > 0 && stream_iter != stream_end) {
        if ((*stream_iter)->OpCode() == InstrCode::NOP) {
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

bool Instr::Stream::Iterator::LookingAt(const std::initializer_list<InstrCode> pattern) {
  if (0 == pattern.size()) {
    return true;
  }
  auto counter = 0u;
  Instr *current = m.current;
  auto pattern_iter = pattern.begin();
  const auto pattern_end = pattern.end();
  while (current && pattern_iter != pattern_end && current->OpCode() == *pattern_iter) {
    current = current->Next();
    ++pattern_iter;
    ++counter;
  }
  return counter == pattern.size();
}
