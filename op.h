#ifndef BF_CC_OP_H
#define BF_CC_OP_H 1

#include <cstdio>

#include "base.h"
#include "machine.h"

enum class Instr {
  INCR_CELL     = 1 << 0, // 1
  DECR_CELL     = 1 << 1, // 2
  SET_CELL      = 1 << 2, // 4
  INCR_PTR      = 1 << 3, // 8
  DECR_PTR      = 1 << 4, // 16
  SET_PTR       = 1 << 5, // 32
  READ          = 1 << 6, // 64
  WRITE         = 1 << 7, // 128
  JUMP_ZERO     = 1 << 8, // 256
  JUMP_NON_ZERO = 1 << 9, // 512
};

class Op final {
private:
  struct M {
    Instr instr;
    s32 operand1;
    s32 operand2;
  } m;

  explicit Op(M m)
    : m(std::move(m))
  {}

public:

  inline Instr Cmd() const {
    return m.instr;
  }

  inline bool IsJump() const {
    return m.instr == Instr::JUMP_ZERO || m.instr == Instr::JUMP_NON_ZERO;
  }

  inline bool IsIO() const {
    return m.instr == Instr::READ || m.instr == Instr::WRITE;
  }

  inline void Exec(MachineState& state) const {
    switch (m.instr) {
    case Instr::INCR_CELL: {
      state.IncrementCell(m.operand1);
    } break;
    case Instr::DECR_CELL: {
      state.DecrementCell(m.operand1);
    } break;
    case Instr::SET_CELL: {
      state.SetCell((u8) m.operand1);
    } break;
    case Instr::INCR_PTR: {
      state.IncrementDataPointer(m.operand1);
    } break;
    case Instr::DECR_PTR: {
      state.DecrementDataPointer(m.operand1);
    } break;
    case Instr::SET_PTR: {
      state.SetDataPointer(m.operand1);
    } break;
    case Instr::READ: {
      u8 input = (u8) std::getchar();
      state.SetCell(input);
    } break;
    case Instr::WRITE: {
      u8 output = state.GetCell();
      std::putchar((int) output);
    } break;
    case Instr::JUMP_ZERO: {
      if (state.GetCell() == 0) {
        if (m.operand2 > 0) {
          state.IncrementInstructionPointer(m.operand1);
        } else {
          state.DecrementInstructionPointer(m.operand1);
        }
      }
    } break;
    case Instr::JUMP_NON_ZERO: {
      if (state.GetCell() != 0) {
        if (m.operand2 > 0) {
          state.IncrementInstructionPointer(m.operand1);
        } else {
          state.DecrementInstructionPointer(m.operand1);
        }
      }
    } break;
    }
  }

  static Op Create(Instr instr, s32 op1 = 0, s32 op2 = 0) {
    return Op(M{
        .instr = instr,
        .operand1 = op1,
        .operand2 = op2,
    });
  }
};

#endif
