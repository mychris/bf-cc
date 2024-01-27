// SPDX-License-Identifier: MIT License
#include "interp.h"

#include <cstdint>
#include <cstdio>

#include "mem.h"
#include "instr.h"

void Interpreter::Run(Heap &heap, Instr::Stream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    switch (iter->OpCode()) {
    case InstrCode::ANY: {
      // TODO: should not be in the stream!
    } break;
    case InstrCode::NOP: {
    } break;
    case InstrCode::INCR_CELL: {
      heap.IncrementCell((uint8_t) iter->Operand1(), iter->Operand2());
    } break;
    case InstrCode::DECR_CELL: {
      heap.DecrementCell((uint8_t) iter->Operand1(), iter->Operand2());
    } break;
    case InstrCode::SET_CELL: {
      heap.SetCell((uint8_t) iter->Operand1(), iter->Operand2());
    } break;
    case InstrCode::INCR_PTR: {
      heap.IncrementDataPointer((int64_t) iter->Operand1());
    } break;
    case InstrCode::DECR_PTR: {
      heap.DecrementDataPointer((int64_t) iter->Operand1());
    } break;
    case InstrCode::READ: {
      const int input = std::getchar();
      if (EOF == input) {
        heap.SetCell(0, iter->Operand2());
      } else {
        heap.SetCell((uint8_t) input, iter->Operand2());
      }
    } break;
    case InstrCode::WRITE: {
      const uint8_t output = heap.GetCell(iter->Operand2());
      std::putchar((int) output);
    } break;
    case InstrCode::JUMP_ZERO: {
      if (heap.GetCell(0) == 0) {
        iter.JumpTo((Instr *) iter->Operand1());
      }
    } break;
    case InstrCode::JUMP_NON_ZERO: {
      if (heap.GetCell(0) != 0) {
        iter.JumpTo((Instr *) iter->Operand1());
      }
    } break;
    case InstrCode::FIND_CELL_HIGH: {
      const uint8_t val = (uint8_t) iter->Operand1();
      while (heap.GetCell(0) != val) {
        heap.IncrementDataPointer((int64_t) iter->Operand2());
      }
    } break;
    case InstrCode::FIND_CELL_LOW: {
      const uint8_t val = (uint8_t) iter->Operand1();
      while (heap.GetCell(0) != val) {
        heap.DecrementDataPointer((int64_t) iter->Operand2());
      }
    } break;
    }
    ++iter;
  }
}
