// SPDX-License-Identifier: MIT License
#include "instr.h"
#include "optimize.h"

static void ReplaceSingleInstructionLoops(OperationStream &stream) {
  static const auto incr_pattern = {Instruction::JUMP_ZERO, Instruction::INCR_CELL, Instruction::JUMP_NON_ZERO};
  static const auto decr_pattern = {Instruction::JUMP_ZERO, Instruction::DECR_CELL, Instruction::JUMP_NON_ZERO};
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    if (iter.LookingAt(incr_pattern) || iter.LookingAt(decr_pattern)) {
      auto first = iter++;
      auto second = iter++;
      auto third = iter++;
      if (first->Operand1() == (Operation::operand_type) *third && third->Operand1() == (Operation::operand_type) *first
          && second->Operand1() % 2 == 1) {
        first->SetOpCode(Instruction::SET_CELL);
        first->SetOperand1(0);
        stream.Delete(second);
        stream.Delete(third);
      }
    } else {
      ++iter;
    }
  }
}

static void ReplaceFindCellLoops(OperationStream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    if (iter->IsJump()) {
      Operation *first = (*iter++);
      Operation *second = *iter;
      Operation *third = (second) ? *(iter + 1) : nullptr;
      if (first && second && third && first->Is(Instruction::JUMP_ZERO) && third->Is(Instruction::JUMP_NON_ZERO)
          && first->Operand1() == (Operation::operand_type) third
          && third->Operand1() == (Operation::operand_type) first) {
        bool replaced = false;
        if (second->Is(Instruction::INCR_PTR)) {
          // [>]
          first->SetOpCode(Instruction::FIND_CELL_HIGH);
          first->SetOperand1(0);
          first->SetOperand2(second->Operand1());
          replaced = true;
        } else if (second->Is(Instruction::DECR_PTR)) {
          // [<]
          first->SetOpCode(Instruction::FIND_CELL_LOW);
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

static void MergeSetIncrDecr(OperationStream &stream) {
  static const auto incr_pattern = {Instruction::SET_CELL, Instruction::INCR_CELL};
  static const auto decr_pattern = {Instruction::SET_CELL, Instruction::DECR_CELL};
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    if (iter.LookingAt(incr_pattern)) {
      Operation *set = *iter;
      ++iter;
      Operation *incr = *iter;
      if (set->Operand2() == incr->Operand2()) {
        set->SetOperand1(set->Operand1() + incr->Operand1());
        stream.Delete(iter--);
      }
    } else if (iter.LookingAt(decr_pattern)) {
      Operation *set = *iter;
      ++iter;
      Operation *decr = *iter;
      if (set->Operand2() == decr->Operand2()) {
        set->SetOperand1(set->Operand1() - decr->Operand1());
        stream.Delete(iter--);
      }
    } else {
      ++iter;
    }
  }
}

void OptPeep(OperationStream &stream) {
  ReplaceSingleInstructionLoops(stream);
  ReplaceFindCellLoops(stream);
  MergeSetIncrDecr(stream);
}
