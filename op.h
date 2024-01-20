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
    Op* next;
    uintptr_t operand1;
    uintptr_t operand2;
  } m;

  explicit Op(M m)
    : m(std::move(m))
  {}

public:

  ~Op() = default;

  inline Instr Cmd() const {
    return m.instr;
  }

  inline void SetCmd(Instr cmd) {
    m.instr = cmd;
  }

  inline bool IsJump() const {
    return m.instr == Instr::JUMP_ZERO || m.instr == Instr::JUMP_NON_ZERO;
  }

  inline bool IsIO() const {
    return m.instr == Instr::READ || m.instr == Instr::WRITE;
  }

  inline uintptr_t Operand1() const {
    return m.operand1;
  }

  inline void SetOperand1(uintptr_t val) {
    m.operand1 = val;
  }

  inline s32 Operand2() const {
    return m.operand2;
  }

  inline void SetOperand2(uintptr_t val) {
    m.operand2 = val;
  }

  inline Op* Next() const {
    return m.next;
  }

  inline void SetNext(Op* next) {
    m.next = next;
  }

  inline Op* Exec(MachineState& state) const {
    switch (m.instr) {
    case Instr::NOP: {
      return m.next;
    } break;
    case Instr::INCR_CELL: {
      state.IncrementCell(m.operand1);
      return m.next;
    } break;
    case Instr::DECR_CELL: {
      state.DecrementCell(m.operand1);
      return m.next;
    } break;
    case Instr::SET_CELL: {
      state.SetCell((u8) m.operand1);
      return m.next;
    } break;
    case Instr::INCR_PTR: {
      state.IncrementDataPointer(m.operand1);
      return m.next;
    } break;
    case Instr::DECR_PTR: {
      state.DecrementDataPointer(m.operand1);
      return m.next;
    } break;
    case Instr::SET_PTR: {
      state.SetDataPointer(m.operand1);
      return m.next;
    } break;
    case Instr::READ: {
      u8 input = (u8) std::getchar();
      state.SetCell(input);
      return m.next;
    } break;
    case Instr::WRITE: {
      u8 output = state.GetCell();
      std::putchar((int) output);
      return m.next;
    } break;
    case Instr::JUMP_ZERO: {
      if (state.GetCell() == 0) {
        return (Op*) m.operand1;
      }
      return m.next;
    } break;
    case Instr::JUMP_NON_ZERO: {
      if (state.GetCell() != 0) {
        return (Op*) m.operand1;
      }
      return m.next;
    } break;
    }
  }

  char* Str() const {
    sprintf(buffer, "%d %zu %zu", m.instr, m.operand1, m.operand2);
    return buffer;
  }

  static Op Create(Instr instr, uintptr_t op1 = 0, uintptr_t op2 = 0) {
    return Op(M{
        .instr = instr,
        .next = nullptr,
        .operand1 = op1,
        .operand2 = op2,
    });
  }

  static Op *Allocate(Instr instr, uintptr_t op1 = 0, uintptr_t op2 = 0) {
    return new Op(M{
        .instr = instr,
        .next = nullptr,
        .operand1 = op1,
        .operand2 = op2,
    });
  }
};

#endif
