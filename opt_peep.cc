#include "instr.h"
#include "optimize.h"

Instr *OptPeep::Run(Instr *op) {
  op = RemoveSetZero(op);
  return op;
}

Instr *OptPeep::RemoveSetZero(Instr *op) {
  Instr *head = op;
  while (op) {
    if (op->IsJump()) {
      Instr *first = op;
      Instr *second = op->Next();
      Instr *third = (second) ? second->Next() : nullptr;
      if (first && second && third && first->IsJump() && third->IsJump() &&
          first->Operand1() == (uintptr_t)third &&
          third->Operand1() == (uintptr_t)first &&
          (second->OpCode() == OpCode::DECR_CELL ||
           second->OpCode() == OpCode::INCR_CELL)) {
        first->SetOpCode(OpCode::SET_CELL);
        first->SetOperand1(0);
        first->SetNext(third->Next());
        delete second;
        delete third;
      }
    }
    op = op->Next();
  }
  return head;
}
