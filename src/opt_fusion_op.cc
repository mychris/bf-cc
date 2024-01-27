// SPDX-License-Identifier: MIT License
#include "instr.h"
#include "optimize.h"

void OptFusionOp(OperationStream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    Instruction seq_cmd = (*iter)->OpCode();
    if (seq_cmd == Instruction::INCR_CELL || seq_cmd == Instruction::DECR_CELL || seq_cmd == Instruction::INCR_PTR
        || seq_cmd == Instruction::DECR_PTR) {
      Operation *seq_head = *iter;
      Operation::operand_type amount = (*iter)->Operand1();
      ++iter;
      while (iter != end && (*iter)->OpCode() == seq_cmd) {
        Operation *cur = *iter;
        amount += cur->Operand1();
        stream.Delete(iter++);
        seq_head->SetOperand1(amount);
      }
    } else {
      ++iter;
    }
  }
}
