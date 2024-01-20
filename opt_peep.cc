#include <vector>

#include "op.h"
#include "optimize.h"

Op *OptPeep::Run(Op *op) {
  op = RemoveSetZero(op);
  return op;
}

Op *OptPeep::RemoveSetZero(Op *op) {
  Op *head = op;
  while (op) {
    if (op->IsJump()) {
      Op *first = op;
      Op *second = op->Next();
      Op *third = (second) ? second->Next() : nullptr;
      if (first && second && third && first->IsJump() && third->IsJump() &&
          first->Operand1() == (uintptr_t)third &&
          third->Operand1() == (uintptr_t)first &&
          (second->Cmd() == Instr::DECR_CELL ||
           second->Cmd() == Instr::INCR_CELL)) {
        first->SetCmd(Instr::SET_CELL);
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
