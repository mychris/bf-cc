#include "op.h"
#include "optimize.h"

Op* OptCommentLoop::Run(Op* op)
{
  Op* head = op;
  while (op->Cmd() == OpCode::NOP) {
    op = op->Next();
  }
  Op* comment_loop = op;
  if (comment_loop->IsJump()) {
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
    comment_loop->SetCmd(OpCode::NOP);
    comment_loop->SetOperand1(0);
  }
  return head;
}
