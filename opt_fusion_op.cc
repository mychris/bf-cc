#include "op.h"
#include "optimize.h"

Op* OptFusionOp::Run(Op* op) {
  Op* head = op;
  // First loop, fuse the same operations
  while (op) {
    OpCode seq_cmd = op->OpCode();
    if (seq_cmd == OpCode::INCR_CELL
        || seq_cmd == OpCode::DECR_CELL
        || seq_cmd == OpCode::INCR_PTR
        || seq_cmd == OpCode::DECR_PTR) {
      Op* seq_head = op;
      uintptr_t amount = op->Operand1();
      op = op->Next();
      while (op->OpCode() == seq_cmd) {
        amount += op->Operand1();
        seq_head->SetOperand1(amount);
        seq_head->SetNext(op->Next());
        delete op;
        op = seq_head->Next();
      }
    } else {
      op = op->Next();
    }
  }
  return head;
}
