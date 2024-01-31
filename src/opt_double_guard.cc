// SPDX-License-Identifier: MIT License
#include <cassert>
#include <cstdio>

#include "instr.h"
#include "optimize.h"

/**
 * Remove double guards.
 *
 * Since jumps and labels do not need to be balanced, double guards
 * can be removed.
 *
 * [ [- > + <] ... ]
 *
 * Delayed pointer increment and multiplicative loop optimization helps
 * to remove even more guards.
 */
void OptDoubleGuard(OperationStream &stream) {
  const auto end = stream.End();
  for (auto iter = stream.Begin(); iter != end; ++iter) {
    if (!iter->IsJump()) {
      continue;
    }
    auto cur = iter + 1;
    // Look at all the operations up to the next jump.
    // If there is no pointer movement and no operation is
    // using the current cell, the next jump can be removed.
    while (cur != end) {
      bool do_break = false;
      switch (cur->OpCode()) {
      case Instruction::NOP:
      case Instruction::LABEL:
        break;
      case Instruction::INCR_CELL:
      case Instruction::DECR_CELL:
      case Instruction::IMUL_CELL:
      case Instruction::DMUL_CELL:
      case Instruction::SET_CELL:
      case Instruction::READ:
      case Instruction::WRITE:
        if (0 == cur->Operand2()) {
          do_break = true;
        }
        break;
      case Instruction::INCR_PTR:
      case Instruction::DECR_PTR:
      case Instruction::FIND_CELL_LOW:
      case Instruction::FIND_CELL_HIGH:
        do_break = true;
        break;
      case Instruction::JZ:
      case Instruction::JNZ:
        do_break = true;
        break;
      }
      if (do_break) {
        break;
      }
      ++cur;
    }
    // Jump must be of the same type!
    if (cur != iter && cur != end && cur->Is(iter->OpCode())) {
      auto jump_label = cur;
      jump_label.JumpTo((Operation *) cur->Operand1());
      stream.Delete(cur);
      stream.Delete(jump_label);
    }
  }
}
