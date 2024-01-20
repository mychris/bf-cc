#include "instr.h"
#include "optimize.h"

Instr *OptPeep::Run(Instr *op) {
  op = RemoveSetZero(op);
  return op;
}

Instr *OptPeep::RemoveSetZero(Instr *op) {
  Instr *head = op;
  while (op) {
    // Find loops with a single instruction
    if (op->IsJump()) {
      Instr *first = op;
      Instr *second = op->Next();
      Instr *third = (second) ? second->Next() : nullptr;
      if (first && second && third && first->OpCode() == OpCode::JUMP_ZERO &&
          third->OpCode() == OpCode::JUMP_NON_ZERO &&
          first->Operand1() == (uintptr_t)third &&
          third->Operand1() == (uintptr_t)first) {
        bool replaced = false;
        if (second->OpCode() == OpCode::DECR_CELL ||
            second->OpCode() == OpCode::INCR_CELL) {
          // [+] [-]
          first->SetOpCode(OpCode::SET_CELL);
          first->SetOperand1(0);
          replaced = true;
        } else if (second->OpCode() == OpCode::INCR_PTR) {
          // [>]
          first->SetOpCode(OpCode::FIND_CELL_HIGH);
          first->SetOperand1(0);
          first->SetOperand2(second->Operand1());
          replaced = true;
        } else if (second->OpCode() == OpCode::DECR_PTR) {
          // [<]
          first->SetOpCode(OpCode::FIND_CELL_LOW);
          first->SetOperand1(0);
          first->SetOperand2(second->Operand1());
          replaced = true;
        }
        if (replaced) {
          first->SetNext(third->Next());
          delete second;
          delete third;
        }
      }
    }
    // Merge Set with + or -
    if (op->OpCode() == OpCode::SET_CELL) {
      bool replaced = false;
      if (op->Next() && op->Next()->OpCode() == OpCode::INCR_CELL) {
        u8 value = (u8)op->Operand1() + (u8)op->Next()->Operand1();
        op->SetOperand1((uintptr_t)value);
        replaced = true;
      } else if (op->Next() && op->Next()->OpCode() == OpCode::DECR_CELL) {
        u8 value = (u8)op->Operand1() - (u8)op->Next()->Operand1();
        op->SetOperand1((uintptr_t)value);
        replaced = true;
      }
      if (replaced) {
        Instr* next = op->Next();
        op->SetNext(op->Next()->Next());
        delete next;
      }
    }
    op = op->Next();
  }
  return head;
}
