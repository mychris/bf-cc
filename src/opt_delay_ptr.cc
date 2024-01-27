// SPDX-License-Identifier: MIT License
#include "instr.h"
#include "optimize.h"

static void do_it(Instr::Stream &stream, Instr::Stream::Iterator &iter, const Instr::Stream::Iterator &end) {
  intptr_t offset = 0;
  while (iter != end) {
    switch ((*iter)->OpCode()) {
    case InstrCode::NOP:
    case InstrCode::ANY: {
      ++iter;
    } break;
    case InstrCode::FIND_CELL_LOW:
    case InstrCode::FIND_CELL_HIGH:
    case InstrCode::JUMP_NON_ZERO:
    case InstrCode::JUMP_ZERO: {
      if (offset != 0) {
        InstrCode code = (offset > 0) ? InstrCode::INCR_PTR : InstrCode::DECR_PTR;
        offset = (offset > 0) ? offset : -offset;
        stream.InsertBefore(*iter, code, offset, 0);
      }
      ++iter;
      return;
    } break;
    case InstrCode::SET_CELL:
    case InstrCode::INCR_CELL:
    case InstrCode::DECR_CELL: {
      (*iter)->SetOperand2(offset);
      ++iter;
    } break;
    case InstrCode::INCR_PTR: {
      offset += (*iter)->Operand1();
      stream.Delete(iter++);
    } break;
    case InstrCode::DECR_PTR: {
      offset -= (*iter)->Operand1();
      stream.Delete(iter++);
    } break;
    case InstrCode::WRITE:
    case InstrCode::READ: {
      (*iter)->SetOperand2(offset);
      ++iter;
    } break;
    }
  }
}

void OptDelayPtr(Instr::Stream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    do_it(stream, iter, end);
  }
}
