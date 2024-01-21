#include "instr.h"
#include "optimize.h"

Instr *OptCommentLoop::Run(Instr *op) {
  Instr *head = op;
  while (op->OpCode() == OpCode::NOP) {
    op = op->Next();
  }
  Instr *comment_loop = op;
  if (comment_loop->IsJump()) {
    op = op->Next();
    while (op) {
      bool last = op->IsJump() && op->Operand1() == (uintptr_t)comment_loop;
      comment_loop->SetNext(op->Next());
      delete op;
      op = comment_loop->Next();
      if (last) {
        break;
      }
    }
    comment_loop->SetOpCode(OpCode::NOP);
    comment_loop->SetOperand1(0);
  }
  return head;
}
