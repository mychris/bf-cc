#include <cstdint>

#include "instr.h"
#include "optimize.h"

static Instr *ReplaceSingleInstructionLoops(Instr *op) {
  Instr *head = op;
  while (op) {
    if (op->IsJump()) {
      Instr *first = op;
      Instr *second = op->Next();
      Instr *third = (second) ? second->Next() : nullptr;
      if (first && second && third &&
          first->OpCode() == Instr::Code::JUMP_ZERO &&
          third->OpCode() == Instr::Code::JUMP_NON_ZERO &&
          first->Operand1() == (uintptr_t)third &&
          third->Operand1() == (uintptr_t)first) {
        if (second->OpCode() == Instr::Code::DECR_CELL ||
            second->OpCode() == Instr::Code::INCR_CELL) {
          // [+] [-]
          first->SetOpCode(Instr::Code::SET_CELL);
          first->SetOperand1(0);
          first->SetNext(third->Next());
          delete second;
          delete third;
        }
      }
    }
    op = op->Next();
  }
  return head;
}

static Instr *ReplaceFindCellLoops(Instr *op) {
  Instr *head = op;
  while (op) {
    if (op->IsJump()) {
      Instr *first = op;
      Instr *second = op->Next();
      Instr *third = (second) ? second->Next() : nullptr;
      if (first && second && third &&
          first->OpCode() == Instr::Code::JUMP_ZERO &&
          third->OpCode() == Instr::Code::JUMP_NON_ZERO &&
          first->Operand1() == (uintptr_t)third &&
          third->Operand1() == (uintptr_t)first) {
        bool replaced = false;
        if (second->OpCode() == Instr::Code::INCR_PTR) {
          // [>]
          first->SetOpCode(Instr::Code::FIND_CELL_HIGH);
          first->SetOperand1(0);
          first->SetOperand2(second->Operand1());
          replaced = true;
        } else if (second->OpCode() == Instr::Code::DECR_PTR) {
          // [<]
          first->SetOpCode(Instr::Code::FIND_CELL_LOW);
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
    op = op->Next();
  }
  return head;
}

static Instr *MergeSetIncrDecr(Instr *op) {
  Instr *head = op;
  while (op) {
    if (op->OpCode() == Instr::Code::SET_CELL) {
      bool replaced = false;
      uint8_t value = 0;
      if (op->Next() && op->Next()->OpCode() == Instr::Code::INCR_CELL) {
        value = (uint8_t)op->Operand1() + (uint8_t)op->Next()->Operand1();
        op->SetOperand1((uintptr_t)value);
        replaced = true;
      } else if (op->Next() && op->Next()->OpCode() == Instr::Code::DECR_CELL) {
        value = (uint8_t)op->Operand1() - (uint8_t)op->Next()->Operand1();
        op->SetOperand1((uintptr_t)value);
        replaced = true;
      }
      if (replaced) {
        Instr *next = op->Next();
        op->SetNext(op->Next()->Next());
        delete next;
      }
    }
    op = op->Next();
  }
  return head;
}

Instr *OptPeep(Instr *op) {
  op = ReplaceSingleInstructionLoops(op);
  op = ReplaceFindCellLoops(op);
  op = MergeSetIncrDecr(op);
  return op;
}
