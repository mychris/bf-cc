// SPDX-License-Identifier: MIT License
#include "interp.h"

#include <cstdint>
#include <cstdio>

#include "instr.h"
#include "mem.h"

void Interpreter::Run(Heap &heap, OperationStream &stream, EOFMode eof_mode) const {
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    switch (iter->OpCode()) {
    case Instruction::NOP: {
    } break;
    case Instruction::INCR_CELL: {
      heap.IncrementCell((uint8_t) iter->Operand1(), iter->Operand2());
    } break;
    case Instruction::DECR_CELL: {
      heap.DecrementCell((uint8_t) iter->Operand1(), iter->Operand2());
    } break;
    case Instruction::IMUL_CELL: {
      uint8_t cur = heap.GetCell(0);
      cur *= iter->Operand1();
      heap.IncrementCell(cur, iter->Operand2());
    } break;
    case Instruction::SET_CELL: {
      heap.SetCell((uint8_t) iter->Operand1(), iter->Operand2());
    } break;
    case Instruction::INCR_PTR: {
      heap.IncrementDataPointer((int64_t) iter->Operand1());
    } break;
    case Instruction::DECR_PTR: {
      heap.DecrementDataPointer((int64_t) iter->Operand1());
    } break;
    case Instruction::READ: {
      const int input = std::getchar();
      if (EOF == input) {
        switch (eof_mode) {
        case EOFMode::KEEP: {
        } break;
        case EOFMode::ZERO: {
          heap.SetCell(0, iter->Operand2());
        } break;
        case EOFMode::NEG_ONE: {
          heap.SetCell((uint8_t) -1, iter->Operand2());
        } break;
        }
      } else {
        heap.SetCell((uint8_t) input, iter->Operand2());
      }
    } break;
    case Instruction::WRITE: {
      const uint8_t output = heap.GetCell(iter->Operand2());
      std::putchar((int) output);
    } break;
    case Instruction::JUMP_ZERO: {
      if (heap.GetCell(0) == 0) {
        iter.JumpTo((Operation *) iter->Operand1());
      }
    } break;
    case Instruction::JUMP_NON_ZERO: {
      if (heap.GetCell(0) != 0) {
        iter.JumpTo((Operation *) iter->Operand1());
      }
    } break;
    case Instruction::FIND_CELL_HIGH: {
      const uint8_t val = (uint8_t) iter->Operand1();
      while (heap.GetCell(0) != val) {
        heap.IncrementDataPointer((int64_t) iter->Operand2());
      }
    } break;
    case Instruction::FIND_CELL_LOW: {
      const uint8_t val = (uint8_t) iter->Operand1();
      while (heap.GetCell(0) != val) {
        heap.DecrementDataPointer((int64_t) iter->Operand2());
      }
    } break;
    }
    ++iter;
  }
}
