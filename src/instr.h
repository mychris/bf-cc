#ifndef BF_CC_INSTR_H
#define BF_CC_INSTR_H 1

#include <cstdio>

#include "base.h"
#include "machine.h"

enum class OpCode {
  NOP            = 1 << 0,
  INCR_CELL      = 1 << 1,
  DECR_CELL      = 1 << 2,
  SET_CELL       = 1 << 3,
  INCR_PTR       = 1 << 4,
  DECR_PTR       = 1 << 5,
  SET_PTR        = 1 << 6,
  READ           = 1 << 7,
  WRITE          = 1 << 8,
  JUMP_ZERO      = 1 << 9,
  JUMP_NON_ZERO  = 1 << 10,
  FIND_CELL_LOW  = 1 << 11,
  FIND_CELL_HIGH = 1 << 12,
};

static char buffer[1024];

class Instr final {
private:
  struct M {
    OpCode instr;
    Instr *next;
    uintptr_t operand1;
    uintptr_t operand2;
  } m;

  explicit Instr(M m) : m(std::move(m)) {}

public:
  ~Instr() = default;

  inline OpCode OpCode() const { return m.instr; }

  inline void SetOpCode(enum OpCode cmd) { m.instr = cmd; }

  inline bool IsJump() const {
    return m.instr == OpCode::JUMP_ZERO || m.instr == OpCode::JUMP_NON_ZERO;
  }

  inline bool IsIO() const {
    return m.instr == OpCode::READ || m.instr == OpCode::WRITE;
  }

  inline uintptr_t Operand1() const { return m.operand1; }

  inline void SetOperand1(uintptr_t val) { m.operand1 = val; }

  inline s32 Operand2() const { return m.operand2; }

  inline void SetOperand2(uintptr_t val) { m.operand2 = val; }

  inline Instr *Next() const { return m.next; }

  inline void SetNext(Instr *next) { m.next = next; }

  char *Str() const {
    sprintf(buffer, "%d %zu %zu", m.instr, m.operand1, m.operand2);
    return buffer;
  }

  static Instr Create(enum OpCode instr, uintptr_t op1 = 0, uintptr_t op2 = 0) {
    return Instr(M{
        .instr = instr,
        .next = nullptr,
        .operand1 = op1,
        .operand2 = op2,
    });
  }

  static Instr *Allocate(enum OpCode instr, uintptr_t op1 = 0, uintptr_t op2 = 0) {
    return new Instr(M{
        .instr = instr,
        .next = nullptr,
        .operand1 = op1,
        .operand2 = op2,
    });
  }
};

#endif
