// SPDX-License-Identifier: MIT License
#include <cassert>
#include <cstdio>

#include "instr.h"
#include "optimize.h"

void OptCommentLoop(Operation::Stream &stream) {
  auto iter = stream.Begin();
  auto end = stream.End();
  while (iter != end && iter->OpCode() == Instruction::NOP) {
    ++iter;
  }
  if (iter == end || !iter->IsJump()) {
    return;
  }
  bool last = false;
  auto comment_loop = *iter;
  ++iter;
  while (iter != end) {
    last = iter->IsJump() && iter->Operand1() == (Operation::operand_type) comment_loop;
    stream.Delete(iter++);
    if (last) {
      break;
    }
  }
  assert(last && "Did not find the end of the comment loop?");
  comment_loop->SetOpCode(Instruction::NOP);
  comment_loop->SetOperand1(0);
}
