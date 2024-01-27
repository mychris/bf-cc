// SPDX-License-Identifier: MIT License
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
    while (iter != end && (iter->OpCode() == Instruction::NOP)) {
      ++iter;
    }
    if (iter == end || !iter->IsJump()) {
      return;
    }
    auto comment_loop = iter++;
    while (iter != end) {
      bool last = iter->IsJump() && iter->Operand1() == (Operation::operand_type) *comment_loop;
      stream.Delete(iter++);
      if (last) {
        stream.Delete(comment_loop);
        break;
      }
    }
  }
}
