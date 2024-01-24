// SPDX-License-Identifier: MIT License
#include "instr.h"
#include "optimize.h"

void OptFusionOp(Instr::Stream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    Instr::Code seq_cmd = (*iter)->OpCode();
    if (seq_cmd == Instr::Code::INCR_CELL || seq_cmd == Instr::Code::DECR_CELL || seq_cmd == Instr::Code::INCR_PTR
        || seq_cmd == Instr::Code::DECR_PTR) {
      Instr *seq_head = *iter;
      uintptr_t amount = (*iter)->Operand1();
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
