// SPDX-License-Identifier: MIT License
#include "instr.h"
#include "optimize.h"

/**
 * Fuse operations.
 *
 * Fuses consecutive increment and decrement operations for the pointer
 * and the current cell.  Does not merge together different instructions.
 */
void OptFusionOp(OperationStream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  // Split between incr/decr of cells and pointer, because of potential
  // increment delay optimization.
  // Should run afterwards, but just in case.
  while (iter != end) {
    Instruction seq_cmd = iter->OpCode();
    if (seq_cmd == Instruction::INCR_CELL || seq_cmd == Instruction::DECR_CELL) {
      auto seq_head = iter++;
      Operation::operand_type amount = seq_head->Operand1();
      while (iter != end && iter->OpCode() == seq_cmd && iter->Operand2() == seq_head->Operand2()) {
        amount += iter->Operand1();
        stream.Delete(iter++);
      }
      amount = amount % 256;
      if (amount == 0) {
        stream.Delete(seq_head);
      } else {
        seq_head->SetOperand1(amount % 256);
      }
    } else if (seq_cmd == Instruction::INCR_PTR || seq_cmd == Instruction::DECR_PTR) {
      auto seq_head = iter++;
      Operation::operand_type amount = seq_head->Operand1();
      while (iter != end && iter->OpCode() == seq_cmd) {
        amount += iter->Operand1();
        stream.Delete(iter++);
      }
      amount = amount % 256;
      seq_head->SetOperand1(amount % 256);
    } else {
      ++iter;
    }
  }
}
