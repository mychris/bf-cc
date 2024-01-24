// SPDX-License-Identifier: MIT License
#include <cstdint>

#include "instr.h"
#include "optimize.h"

static void ReplaceSingleInstructionLoops(Instr::Stream &stream) {
  auto callback = [](Instr *instr) {
    Instr *first = instr;
    Instr *second = first->Next();
    Instr *third = second->Next();
    if (first->Operand1() == (uintptr_t) third && third->Operand1() == (uintptr_t) first &&
        second->Operand1() % 2 == 1) {
          first->SetOpCode(Instr::Code::SET_CELL);
          first->SetOperand1(0);
          second->SetOpCode(Instr::Code::NOP);
          third->SetOpCode(Instr::Code::NOP);
    }
  };
  stream.VisitPattern({Instr::Code::JUMP_ZERO, Instr::Code::INCR_CELL, Instr::Code::JUMP_NON_ZERO}, callback);
  stream.VisitPattern({Instr::Code::JUMP_ZERO, Instr::Code::DECR_CELL, Instr::Code::JUMP_NON_ZERO}, callback);
}

static void ReplaceFindCellLoops(Instr::Stream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    if ((*iter)->IsJump()) {
      Instr *first = (*iter++);
      Instr *second = first->Next();
      Instr *third = (second) ? second->Next() : nullptr;
      if (first && second && third && first->OpCode() == Instr::Code::JUMP_ZERO
          && third->OpCode() == Instr::Code::JUMP_NON_ZERO && first->Operand1() == (uintptr_t) third
          && third->Operand1() == (uintptr_t) first) {
        bool replaced = false;
        if (second->OpCode() == Instr::Code::INCR_PTR) {
          // [>]
          first->SetOpCode(Instr::Code::FIND_CELL_HIGH);
          first->SetOperand1(0);
          first->SetOperand2(second->Operand1());
          replaced = true;
        } else if (second->OpCode() == Instr::Code::DECR_PTR) {
          // [<]
          first->SetOpCode(Instr::Code::FIND_CELL_LOW);
          first->SetOperand1(0);
          first->SetOperand2(second->Operand1());
          replaced = true;
        }
        if (replaced) {
          stream.Delete(iter++);
          stream.Delete(iter++);
        }
      }
    } else {
      ++iter;
    }
  }
}

static void MergeSetIncrDecr(Instr::Stream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    if ((*iter)->OpCode() == Instr::Code::SET_CELL) {
      Instr *instr = *iter;
      Instr::Code next_op_code = Instr::Code::NOP;
      if (instr->Next()) {
        next_op_code = instr->Next()->OpCode();
      }
      uint8_t value = 0;
      if (next_op_code == Instr::Code::INCR_CELL || next_op_code == Instr::Code::DECR_CELL) {
        value = (uint8_t) instr->Operand1();
        if (next_op_code == Instr::Code::INCR_CELL) {
          value += (uint8_t) instr->Next()->Operand1();
        } else {
          value -= (uint8_t) instr->Next()->Operand1();
        }
        instr->SetOperand1(value);
        stream.Delete(++iter);
      } else {
        ++iter;
      }
    } else {
      ++iter;
    }
  }
}

void OptPeep(Instr::Stream &stream) {
  ReplaceSingleInstructionLoops(stream);
  ReplaceFindCellLoops(stream);
  MergeSetIncrDecr(stream);
}
