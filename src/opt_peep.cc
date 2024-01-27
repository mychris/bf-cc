// SPDX-License-Identifier: MIT License
#include <cstdint>

#include "instr.h"
#include "optimize.h"

static void ReplaceSingleInstructionLoops(Instr::Stream &stream) {
  auto callback = [](Instr *instr) {
    Instr *first = instr;
    Instr *second = first->Next();
    Instr *third = second->Next();
    if (first->Operand1() == (Instr::operand_type) third && third->Operand1() == (Instr::operand_type) first &&
        second->Operand1() % 2 == 1) {
          first->SetOpCode(InstrCode::SET_CELL);
          first->SetOperand1(0);
          second->SetOpCode(InstrCode::NOP);
          third->SetOpCode(InstrCode::NOP);
    }
  };
  stream.VisitPattern({InstrCode::JUMP_ZERO, InstrCode::INCR_CELL, InstrCode::JUMP_NON_ZERO}, callback);
  stream.VisitPattern({InstrCode::JUMP_ZERO, InstrCode::DECR_CELL, InstrCode::JUMP_NON_ZERO}, callback);
}

static void ReplaceFindCellLoops(Instr::Stream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    if ((*iter)->IsJump()) {
      Instr *first = (*iter++);
      Instr *second = first->Next();
      Instr *third = (second) ? second->Next() : nullptr;
      if (first && second && third && first->OpCode() == InstrCode::JUMP_ZERO
          && third->OpCode() == InstrCode::JUMP_NON_ZERO && first->Operand1() == (Instr::operand_type) third
          && third->Operand1() == (Instr::operand_type) first) {
        bool replaced = false;
        if (second->OpCode() == InstrCode::INCR_PTR) {
          // [>]
          first->SetOpCode(InstrCode::FIND_CELL_HIGH);
          first->SetOperand1(0);
          first->SetOperand2(second->Operand1());
          replaced = true;
        } else if (second->OpCode() == InstrCode::DECR_PTR) {
          // [<]
          first->SetOpCode(InstrCode::FIND_CELL_LOW);
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
  auto callback = [](Instr *instr) {
    Instr *set = instr;
    Instr *incr_decr = set->Next();
    if (incr_decr->OpCode() == InstrCode::INCR_CELL) {
      set->SetOperand1(set->Operand1() + incr_decr->Operand1());
    } else {
      set->SetOperand1(set->Operand1() - incr_decr->Operand1());
    }
    incr_decr->SetOpCode(InstrCode::NOP);
  };
  stream.VisitPattern({InstrCode::SET_CELL, InstrCode::INCR_CELL}, callback);
  stream.VisitPattern({InstrCode::SET_CELL, InstrCode::DECR_CELL}, callback);
}

void OptPeep(Instr::Stream &stream) {
  ReplaceSingleInstructionLoops(stream);
  ReplaceFindCellLoops(stream);
  MergeSetIncrDecr(stream);
}
