// SPDX-License-Identifier: MIT License
#include <cassert>

#include "instr.h"
#include "optimize.h"

/**
 * Optimize comment loop.
 *
 * Removes all comment loops at the beginning of the program.
 * This reduces the code size of the resulting program and can make
 * the optimizer faster, in case the comment loop contains a lot of
 * operations.
 */
void OptCommentLoop(OperationStream &stream) {
  auto iter = stream.Begin();
  auto end = stream.End();
  for (;;) {
    while (iter != end && (iter->Is(Instruction::NOP))) {
      ++iter;
    }
    if (iter == end || !iter->Is(Instruction::JZ)) {
      return;
    }
    auto loop_end = iter;
    loop_end.JumpTo((Operation *) iter->Operand1());
    assert(loop_end->Is(Instruction::LABEL));
    assert((loop_end - 1)->Is(Instruction::JNZ));
    while (iter != loop_end) {
      stream.Delete(iter++);
    }
    stream.Delete(iter++);
  }
}
