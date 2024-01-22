// SPDX-License-Identifier: MIT License
#ifndef BF_CC_INSTR_H
#define BF_CC_INSTR_H 1

#include <stdint.h>

#include <utility>

#include "error.h"

class Instr final {
public:
  enum class Code {
    NOP = 1 << 0,
    INCR_CELL = 1 << 1,
    DECR_CELL = 1 << 2,
    SET_CELL = 1 << 3,
    INCR_PTR = 1 << 4,
    DECR_PTR = 1 << 5,
    // SET_PTR = 1 << 6,
    READ = 1 << 7,
    WRITE = 1 << 8,
    JUMP_ZERO = 1 << 9,
    JUMP_NON_ZERO = 1 << 10,
    FIND_CELL_LOW = 1 << 11,
    FIND_CELL_HIGH = 1 << 12,
  };

private:
  struct M {
    Instr::Code op_code;
    Instr *next;
    uintptr_t operand1;
    uintptr_t operand2;
  } m;

  Instr(const Instr &) = delete;
  Instr &operator=(const Instr &) = delete;

  explicit Instr(M m) : m(std::move(m)) {
  }

public:
  Instr(Instr &&other) : m(std::exchange(other.m, {Instr::Code::NOP, nullptr, 0, 0})) {
  }

  ~Instr() = default;

  inline Instr::Code OpCode() const {
    return m.op_code;
  }

  inline void SetOpCode(enum Instr::Code cmd) {
    m.op_code = cmd;
  }

  inline bool IsJump() const {
    return m.op_code == Instr::Code::JUMP_ZERO || m.op_code == Instr::Code::JUMP_NON_ZERO;
  }

  inline bool IsIO() const {
    return m.op_code == Instr::Code::READ || m.op_code == Instr::Code::WRITE;
  }

  inline uintptr_t Operand1() const {
    return m.operand1;
  }

  inline void SetOperand1(uintptr_t val) {
    m.operand1 = val;
  }

  inline uintptr_t Operand2() const {
    return m.operand2;
  }

  inline void SetOperand2(uintptr_t val) {
    m.operand2 = val;
  }

  inline Instr *Next() const {
    return m.next;
  }

  inline void SetNext(Instr *next) {
    m.next = next;
  }

  static Instr Create(enum Instr::Code op_code, uintptr_t op1 = 0, uintptr_t op2 = 0) {
    return Instr(M{
        .op_code = op_code,
        .next = nullptr,
        .operand1 = op1,
        .operand2 = op2,
    });
  }

  static Instr *Allocate(enum Instr::Code op_code, uintptr_t op1 = 0, uintptr_t op2 = 0) {
    Instr *instr = new (std::nothrow) Instr(M{
        .op_code = op_code,
        .next = nullptr,
        .operand1 = op1,
        .operand2 = op2,
    });
    if (!instr) {
      Error(Err::OutOfMemory());
    }
    return instr;
  }
};

#endif /* BF_CC_INSTR_H */
