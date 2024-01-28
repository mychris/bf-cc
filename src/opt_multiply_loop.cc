// SPDX-License-Identifier: MIT License

#include <cassert>

#include "instr.h"
#include "optimize.h"

static void try_optimize_loop(OperationStream &stream,
                              OperationStream::Iterator iter,
                              const OperationStream::Iterator end);

/**
 * Optimize increment multiplication loops
 *
 * Optimizes multiplication loops into their corresponding operation.
 * This requires operation fusion and delayed moves to be applied before.
 */
void OptMultiplyLoop(OperationStream &stream) {
  auto iter = stream.Begin();
  const auto end = stream.End();
  while (iter != end) {
    if (iter->Is(Instruction::JUMP_ZERO)) {
      bool loop_ok = false;
      auto loop_start = iter;
      auto loop_end = iter;
      ++iter;
      // Find the end of the loop.  Stop if another loop starts.
      while (iter != end) {
        if (iter->IsJump()) {
          if (iter->Is(Instruction::JUMP_NON_ZERO) && iter->Operand1() == (intptr_t) *loop_start) {
            loop_ok = true;
            loop_end = iter;
          }
          break;
        }
        ++iter;
      }
      if (iter != end && loop_ok) {
        // go beyond the loop
        ++iter;
        try_optimize_loop(stream, loop_start, loop_end);
      }
    } else {
      ++iter;
    }
  }
}

static void try_optimize_loop(OperationStream &stream,
                              OperationStream::Iterator iter,
                              const OperationStream::Iterator end) {
  assert(iter->Is(Instruction::JUMP_ZERO));
  assert(end->Is(Instruction::JUMP_NON_ZERO));
  unsigned int decr_count = 0;
  unsigned int incr_count = 0;
  // Check for an appropriate loop
  for (auto cur = iter + 1; cur != end; ++cur) {
    if (cur->Is(Instruction::DECR_CELL)) {
      ++decr_count;
    }
    if (cur->Is(Instruction::INCR_CELL)) {
      ++incr_count;
    }
    // Only + or - operations are allowed
    if (!cur->IsAny({Instruction::INCR_CELL, Instruction::DECR_CELL})) {
      return;
    }
    // The - operation is only allowed for cell 0 with an amount of 1
    if (cur->Is(Instruction::DECR_CELL) && 1 != cur->Operand1() && 0 != cur->Operand2()) {
      return;
    }
    // The + operation is not allowed for cell 0 or with a non positive increment
    if (cur->Is(Instruction::INCR_CELL) && (0 >= cur->Operand1() || 0 == cur->Operand2())) {
      return;
    }
  }
  if (decr_count != 1 || incr_count == 0) {
    return;
  }
  // Change the - operation into a set 0
  // and the + operations into a multiply
  // The set needs to be at the end
  // The loop (guard) needs to stay, otherwise the memory access
  // might get out of bounds, if the program exploits that.
  iter++;
  while (iter != end) {
    if (iter->Is(Instruction::DECR_CELL)) {
      stream.Delete(iter++);
    } else if (iter->Is(Instruction::INCR_CELL)) {
      iter->SetOpCode(Instruction::IMUL_CELL);
      iter++;
    }
  }
  stream.InsertBefore(*iter, Instruction::SET_CELL, 0);
}
