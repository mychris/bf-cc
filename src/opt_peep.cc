// SPDX-License-Identifier: MIT License
#include "debug.h"
#include "instr.h"
#include "optimize.h"

/**
 * Replace loops with a single increment or decrement operation with
 * a set to zero.
 *
 * [+] [-]
 */
static void ReplaceSingleInstructionLoops(OperationStream &stream) {
  static const auto incr_pattern = {
      Instruction::JZ, Instruction::LABEL, Instruction::INCR_CELL, Instruction::JNZ, Instruction::LABEL};
  static const auto decr_pattern = {
      Instruction::JZ, Instruction::LABEL, Instruction::DECR_CELL, Instruction::JNZ, Instruction::LABEL};
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    if (iter.LookingAt(incr_pattern) || iter.LookingAt(decr_pattern)) {
      auto jz = iter++;
      auto label1 = iter++;
      auto incr_decr = iter++;
      auto jnz = iter++;
      auto label2 = iter++;
      if (jz->Operand1() == (Operation::operand_type) *label2 && jnz->Operand1() == (Operation::operand_type) *label1
          && incr_decr->Operand1() % 2 == 1) {
        ASSERT(label2->Operand1() == (Operation::operand_type) *jz, "check");
        ASSERT(label1->Operand1() == (Operation::operand_type) *jnz, "check");
        jz->SetOpCode(Instruction::SET_CELL);
        jz->SetOperand1(0);
        stream.Delete(label1);
        stream.Delete(incr_decr);
        stream.Delete(jnz);
        stream.Delete(label2);
      }
    } else {
      ++iter;
    }
  }
}

/**
 * Replaces loops with a single increment/decrement pointer operation
 * into a find cell operation.
 *
 * [<] [>]
 */
static void ReplaceFindCellLoops(OperationStream &stream) {
  static const auto high_pattern = {
      Instruction::JZ, Instruction::LABEL, Instruction::INCR_PTR, Instruction::JNZ, Instruction::LABEL};
  static const auto low_pattern = {
      Instruction::JZ, Instruction::LABEL, Instruction::DECR_PTR, Instruction::JNZ, Instruction::LABEL};
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    if (iter.LookingAt(high_pattern) || iter.LookingAt(low_pattern)) {
      auto jz = iter++;
      auto label1 = iter++;
      auto incr_decr = iter++;
      auto jnz = iter++;
      auto label2 = iter++;
      if (jz->Operand1() == (Operation::operand_type) *label2 && jnz->Operand1() == (Operation::operand_type) *label1) {
        if (incr_decr->Is(Instruction(Instruction::INCR_PTR))) {
          jz->SetOpCode(Instruction::FIND_CELL_HIGH);
        } else {
          jz->SetOpCode(Instruction::FIND_CELL_LOW);
        }
        jz->SetOperand1(0);
        jz->SetOperand2(incr_decr->Operand1());
        stream.Delete(label1);
        stream.Delete(incr_decr);
        stream.Delete(jnz);
        stream.Delete(label2);
      }
    } else {
      ++iter;
    }
  }
}

/**
 * Merges set cell with following incr/decr cell operations.
 */
static void MergeSetIncrDecr(OperationStream &stream) {
  static const auto incr_pattern = {Instruction::SET_CELL, Instruction::INCR_CELL};
  static const auto decr_pattern = {Instruction::SET_CELL, Instruction::DECR_CELL};
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    if (iter.LookingAt(incr_pattern)) {
      auto set = iter;
      auto incr = iter + 1;
      if (set->Operand2() == incr->Operand2()) {
        set->SetOperand1(set->Operand1() + incr->Operand1());
        stream.Delete(incr);
      }
    } else if (iter.LookingAt(decr_pattern)) {
      auto set = iter;
      auto decr = iter + 1;
      if (set->Operand2() == decr->Operand2()) {
        set->SetOperand1(set->Operand1() - decr->Operand1());
        stream.Delete(decr);
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
