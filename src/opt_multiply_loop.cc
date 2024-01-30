// SPDX-License-Identifier: MIT License

#include <cassert>

#include "instr.h"
#include "optimize.h"

static bool try_optimize_loop(OperationStream &stream,
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
    if (iter->Is(Instruction::JNZ)) {
      bool jump_found = false;
      auto loop_start = iter;
      auto loop_end = iter;
      loop_start.JumpTo((Operation *) loop_end->Operand1());
      auto cur = loop_start;
      assert(loop_start->Is(Instruction::LABEL));
      assert(loop_end->Is(Instruction::JNZ));
      // Find the end of the loop.  Stop if another loop starts.
      while (cur != loop_end) {
        if (cur->IsJump()) {
          jump_found = true;
          break;
        }
        ++cur;
      }
      if (!jump_found) {
        // go beyond the loop
        iter = loop_end + 2;
        if (try_optimize_loop(stream, loop_start, loop_end)) {
          // Delete the backward jump and its label
          stream.Delete(loop_start);
          stream.Delete(loop_end);
        }
      } else {
        ++iter;
      }
    } else {
      ++iter;
    }
  }
}

static inline bool is_loop_counter_decrement(const OperationStream::Iterator &iter) {
  return iter->Is(Instruction::DECR_CELL) && iter->Operand1() == 1 && iter->Operand2() == 0;
}

static bool try_optimize_loop(OperationStream &stream,
                              OperationStream::Iterator iter,
                              const OperationStream::Iterator end) {
  assert(iter->Is(Instruction::LABEL));
  assert(end->Is(Instruction::JNZ));
  unsigned int counter_access_count = 0;
  // Check for an appropriate loop
  for (auto cur = iter + 1; cur != end; ++cur) {
    // Only + or - operations are allowed
    if (!cur->IsAny({Instruction::INCR_CELL, Instruction::DECR_CELL})) {
      return false;
    }
    // only one operation is allowed to access the counter
    // it needs to decrement it by 1
    if (0 == cur->Operand2()) {
      ++counter_access_count;
      if (counter_access_count > 1 || !cur->Is(Instruction::DECR_CELL) || cur->Operand1() != 1) {
        return false;
      }
    }
  }
  if (counter_access_count != 1) {
    return false;
  }
  // Change the - operation into a set 0
  // and the + operations into a multiply
  // The set needs to be at the end
  // The loop (guard) needs to stay, otherwise the memory access
  // might get out of bounds, if the program exploits that.
  auto cur = iter + 1;
  while (cur != end) {
    if (is_loop_counter_decrement(cur)) {
      stream.Delete(cur++);
    } else if (cur->Is(Instruction::INCR_CELL)) {
      cur->SetOpCode(Instruction::IMUL_CELL);
      cur++;
    } else {
      cur->SetOpCode(Instruction::DMUL_CELL);
      cur++;
    }
  }
  stream.InsertBefore(*cur, Instruction::SET_CELL, 0, 0);
  return true;
}
