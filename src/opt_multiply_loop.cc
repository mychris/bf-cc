// SPDX-License-Identifier: MIT License

#include <cassert>

#include "instr.h"
#include "optimize.h"

static void try_optimize_loop(OperationStream &stream,
                              OperationStream::Iterator iter,
                              const OperationStream::Iterator end);

/**
 * Optimize multiplicative loops
 *
 *  [ - > + < ]
 *  [ - > - < ]
 *
 * These kinds of loops increment or decrement cells by a multiple
 * of the loop counter cell.
 *
 * This optimization requires fusion and delayd moves to be applied before.
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

static inline bool is_loop_counter_decrement(const OperationStream::Iterator &iter) {
  return iter->Is(Instruction::DECR_CELL) && iter->Operand1() == 1 && iter->Operand2() == 0;
}

static void try_optimize_loop(OperationStream &stream,
                              OperationStream::Iterator iter,
                              const OperationStream::Iterator end) {
  assert(iter->Is(Instruction::JUMP_ZERO));
  assert(end->Is(Instruction::JUMP_NON_ZERO));
  unsigned int counter_access_count = 0;
  // Check for an appropriate loop
  for (auto cur = iter + 1; cur != end; ++cur) {
    // Only + or - operations are allowed
    if (!cur->IsAny({Instruction::INCR_CELL, Instruction::DECR_CELL})) {
      return;
    }
    // only one operation is allowed to access the counter
    // it needs to decrement it by 1
    if (0 == cur->Operand2()) {
      ++counter_access_count;
      if (counter_access_count > 1 || !cur->Is(Instruction::DECR_CELL) || cur->Operand1() != 1) {
        return;
      }
    }
  }
  if (counter_access_count != 1) {
    return;
  }
  // Change the - operation into a set 0
  // and the + operations into a multiply
  // The set needs to be at the end
  // The loop (guard) needs to stay, otherwise the memory access
  // might get out of bounds, if the program exploits that.
  iter++;
  while (iter != end) {
    if (is_loop_counter_decrement(iter)) {
      stream.Delete(iter++);
    } else if (iter->Is(Instruction::INCR_CELL)) {
      iter->SetOpCode(Instruction::IMUL_CELL);
      iter++;
    } else {
      iter->SetOpCode(Instruction::DMUL_CELL);
      iter++;
    }
  }
  stream.InsertBefore(*iter, Instruction::SET_CELL, 0, 0);
}
