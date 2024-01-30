// SPDX-License-Identifier: MIT License
#include "instr.h"
#include "optimize.h"

void OptDelayPtr(OperationStream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  intptr_t offset = 0;
  while (iter != end) {
    switch (iter->OpCode()) {
    case Instruction::NOP: {
      ++iter;
    } break;
    case Instruction::DMUL_CELL:
    case Instruction::IMUL_CELL:
    case Instruction::FIND_CELL_LOW:
    case Instruction::FIND_CELL_HIGH:
    case Instruction::JZ:
    case Instruction::JNZ:
    case Instruction::LABEL: {
      if (offset != 0) {
        Instruction code = (offset > 0) ? Instruction::INCR_PTR : Instruction::DECR_PTR;
        offset = (offset > 0) ? offset : -offset;
        stream.InsertBefore(*iter, code, offset, 0);
        offset = 0;
      }
      ++iter;
    } break;
    case Instruction::SET_CELL:
    case Instruction::INCR_CELL:
    case Instruction::DECR_CELL: {
      iter->SetOperand2(offset);
      ++iter;
    } break;
    case Instruction::INCR_PTR: {
      offset += iter->Operand1();
      stream.Delete(iter++);
    } break;
    case Instruction::DECR_PTR: {
      offset -= iter->Operand1();
      stream.Delete(iter++);
    } break;
    case Instruction::WRITE:
    case Instruction::READ: {
      iter->SetOperand2(offset);
      ++iter;
    } break;
    }
  }
}
