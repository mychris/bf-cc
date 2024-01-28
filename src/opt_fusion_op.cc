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
      Operation *seq_head = *iter;
      Operation::operand_type amount = iter->Operand1();
      ++iter;
      while (iter != end && iter->OpCode() == seq_cmd && iter->Operand2() == seq_head->Operand2()) {
        Operation *cur = *iter;
        amount += cur->Operand1() % 256;
        stream.Delete(iter++);
        seq_head->SetOperand1(amount);
      }
    } else if (seq_cmd == Instruction::INCR_PTR || seq_cmd == Instruction::DECR_PTR) {
      Operation *seq_head = *iter;
      Operation::operand_type amount = iter->Operand1();
      ++iter;
      while (iter != end && iter->OpCode() == seq_cmd) {
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
