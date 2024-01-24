// SPDX-License-Identifier: MIT License
#include <cassert>

#include <cstdio>

#include "instr.h"
#include "optimize.h"

void OptCommentLoop(Instr::Stream &stream) {
  auto iter = stream.Begin();
  auto end = stream.End();
  while (iter != end && (*iter)->OpCode() == Instr::Code::NOP) {
    ++iter;
  }
  if (iter == end || !(*iter)->IsJump()) {
    return;
  }
  bool last = false;
  auto comment_loop = *iter;
  ++iter;
  while (iter != end) {
    last = (*iter)->IsJump() && (*iter)->Operand1() == (uintptr_t) comment_loop;
    stream.Delete(iter++);
    if (last) {
      break;
    }
  }
  assert(last && "Did not find the end of the comment loop?");
  comment_loop->SetOpCode(Instr::Code::NOP);
  comment_loop->SetOperand1(0);
}
