#include "instr.h"
#include "optimize.h"

Instr* OptFusionOp::Run(Instr* op) {
  Instr* head = op;
  if (op->OpCode() != OpCode::NOP) {
    head = Instr::Allocate(OpCode::NOP, 0);
    head->SetNext(op);
  }
  while (op) {
    OpCode seq_cmd = op->OpCode();
    if (seq_cmd == OpCode::INCR_CELL
        || seq_cmd == OpCode::DECR_CELL
        || seq_cmd == OpCode::INCR_PTR
        || seq_cmd == OpCode::DECR_PTR) {
      Instr* seq_head = op;
      uintptr_t amount = op->Operand1();
      op = op->Next();
      while (op && op->OpCode() == seq_cmd) {
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
