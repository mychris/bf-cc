#include "instr.h"
#include "optimize.h"

Instr *OptFusionOp(Instr *op) {
  Instr *head = op;
  if (op->OpCode() != Instr::Code::NOP) {
    head = Instr::Allocate(Instr::Code::NOP, 0);
    head->SetNext(op);
  }
  while (op) {
    Instr::Code seq_cmd = op->OpCode();
    if (seq_cmd == Instr::Code::INCR_CELL || seq_cmd == Instr::Code::DECR_CELL || seq_cmd == Instr::Code::INCR_PTR
        || seq_cmd == Instr::Code::DECR_PTR) {
      Instr *seq_head = op;
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
