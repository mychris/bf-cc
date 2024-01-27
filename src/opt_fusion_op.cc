// SPDX-License-Identifier: MIT License
#include "instr.h"
#include "optimize.h"

void OptFusionOp(Instr::Stream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    InstrCode seq_cmd = (*iter)->OpCode();
    if (seq_cmd == InstrCode::INCR_CELL || seq_cmd == InstrCode::DECR_CELL || seq_cmd == InstrCode::INCR_PTR
        || seq_cmd == InstrCode::DECR_PTR) {
      Instr *seq_head = *iter;
      Instr::operand_type amount = (*iter)->Operand1();
      ++iter;
      while (iter != end && (*iter)->OpCode() == seq_cmd) {
        Instr *cur = *iter;
        amount += cur->Operand1();
        stream.Delete(iter++);
        seq_head->SetOperand1(amount);
      }
    } else {
      ++iter;
    }
  }
}
