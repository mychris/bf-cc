// SPDX-License-Identifier: MIT License
#include "instr.h"
#include "optimize.h"

Instr *OptCommentLoop(Instr *op) {
  Instr *head = op;
  while (op && op->OpCode() == Instr::Code::NOP) {
    op = op->Next();
  }
  Instr *comment_loop = op;
  if (comment_loop && comment_loop->IsJump()) {
    op = op->Next();
    while (op) {
      bool last = op->IsJump() && op->Operand1() == (uintptr_t) comment_loop;
      comment_loop->SetNext(op->Next());
      delete op;
      op = comment_loop->Next();
      if (last) {
        break;
      }
    }
    comment_loop->SetOpCode(Instr::Code::NOP);
    comment_loop->SetOperand1(0);
  }
  return head;
}
