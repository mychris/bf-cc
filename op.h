#ifndef BF_CC_OP_H
#define BF_CC_OP_H 1

#include <cstdio>

#include "base.h"
#include "machine.h"

enum class Instr {
  NOP           = 1 << 0,
  INCR_CELL     = 1 << 1,
  DECR_CELL     = 1 << 2,
  SET_CELL      = 1 << 3,
  INCR_PTR      = 1 << 4,
  DECR_PTR      = 1 << 5,
  SET_PTR       = 1 << 6,
  READ          = 1 << 7,
  WRITE         = 1 << 8,
  JUMP_ZERO     = 1 << 9,
  JUMP_NON_ZERO = 1 << 10,
};

static char buffer[1024];

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

  ~Op() = default;

  inline Instr Cmd() const {
    return m.instr;
  }

  inline bool IsJump() const {
    return m.instr == Instr::JUMP_ZERO || m.instr == Instr::JUMP_NON_ZERO;
  }

  inline bool IsIO() const {
    return m.instr == Instr::READ || m.instr == Instr::WRITE;
  }

  inline s32 Operand1() const {
    return m.operand1;
  }

  inline s32 Operand2() const {
    return m.operand2;
  }

  inline void Exec(MachineState& state) const {
    switch (m.instr) {
    case Instr::NOP: {
    } break;
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
        state.SetInstructionPointer(m.operand1);
      }
    } break;
    case Instr::JUMP_NON_ZERO: {
      if (state.GetCell() != 0) {
        state.SetInstructionPointer(m.operand1);
      }
    } break;
    }
  }

  char* Str() const {
    sprintf(buffer, "%d %d %d", m.instr, m.operand1, m.operand2);
    return buffer;
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
