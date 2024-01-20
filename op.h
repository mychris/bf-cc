#ifndef BF_CC_OP_H
#define BF_CC_OP_H 1

#include "base.h"
#include "machine.h"

enum class Instr {
  INCR_CELL,
  DECR_CELL,
  SET_CELL,
  INCR_PTR,
  DECR_PTR,
  SET_PTR,
  READ,
  WRITE,
  JUMP_ZERO,
  JUMP_NON_ZERO,
};

class Op {

public:
  virtual Instr Cmd() const = 0;
  virtual void Exec(MachineState&) const = 0;
  virtual ~Op() {}
  Op() = default;

  bool IsJump() const {
    return Cmd() == Instr::JUMP_ZERO || Cmd() == Instr::JUMP_NON_ZERO;
  }

  bool IsIO() const {
    return Cmd() == Instr::READ || Cmd() == Instr::WRITE;
  }

};

class OpIncrCell : public Op {
private:
  struct M {
    u32 operand;
  } m;

  explicit OpIncrCell(M m)
    : m(std::move(m))
  {}

public:
  virtual Instr Cmd() const override {
    return Instr::INCR_CELL;
  }

  virtual void Exec(MachineState& state) const override {
    state.IncrementCell(m.operand);
  }

  static OpIncrCell Create(u32 operand = 1) {
    return OpIncrCell(M{
        .operand = operand,
    });
  }
};

class OpDecrCell : public Op {
private:
  struct M {
    u32 operand;
  } m;

  explicit OpDecrCell(M m)
    : m(std::move(m))
  {}

public:
  virtual Instr Cmd() const override {
    return Instr::DECR_CELL;
  }

  virtual void Exec(MachineState& state) const override {
    state.DecrementCell(m.operand);
  }

  static OpDecrCell Create(u32 operand = 1) {
    return OpDecrCell(M{
        .operand = operand,
    });
  }
};

class OpSetCell : public Op {
private:
  struct M {
    u8 operand;
  } m;

  explicit OpSetCell(M m)
    : m(std::move(m))
  {}

public:
  virtual Instr Cmd() const override {
    return Instr::SET_CELL;
  }

  virtual void Exec(MachineState& state) const override {
    state.SetCell(m.operand);
  }

  static OpSetCell create(u8 operand = 1) {
    return OpSetCell(M{
        .operand = operand,
    });
  }
 
};

class OpIncrPtr : public Op {
private:
  struct M {
    u32 operand;
  } m;

  explicit OpIncrPtr(M m)
    : m(std::move(m))
  {}

public:
  virtual Instr Cmd() const override {
    return Instr::INCR_PTR;
  }

  virtual void Exec(MachineState& state) const override {
    state.IncrementDataPointer(m.operand);
  }

  static OpIncrPtr Create(u32 operand = 1) {
    return OpIncrPtr(M{
        .operand = operand,
    });
  }
};

class OpDecrPtr : public Op {
private:
  struct M {
    u32 operand;
  } m;

  explicit OpDecrPtr(M m)
    : m(std::move(m))
  {}

public:
  virtual Instr Cmd() const override {
    return Instr::DECR_PTR;
  }

  virtual void Exec(MachineState& state) const override {
    state.DecrementDataPointer(m.operand);
  }

  static OpDecrPtr Create(u32 operand = 1) {
    return OpDecrPtr(M{
        .operand = operand,
    });
  }
};

class OpSetPtr : public Op {
private:
  struct M {
    u32 operand;
  } m;

  explicit OpSetPtr(M m)
    : m(std::move(m))
  {}

public:
  virtual Instr Cmd() const override {
    return Instr::SET_PTR;
  }

  virtual void Exec(MachineState& state) const override {
    state.SetDataPointer(m.operand);
  }

  static OpSetPtr Create(u32 operand = 1) {
    return OpSetPtr(M{
        .operand = operand,
    });
  }
};

class OpRead : public Op {
private:
  struct M {
  } m;

  explicit OpRead(M m)
    : m(std::move(m))
  {}

public:
  virtual Instr Cmd() const override {
    return Instr::READ;
  }

  virtual void Exec(MachineState& state) const override {
    u8 input = (u8) std::getchar();
    state.SetCell(input);
  }

  static OpRead Create() {
    return OpRead(M{});
  }
};

class OpWrite : public Op {
private:
  struct M {
  } m;

  explicit OpWrite(M m)
    : m(std::move(m))
  {}

public:
  virtual Instr Cmd() const override {
    return Instr::READ;
  }

  virtual void Exec(MachineState& state) const override {
    u8 output = state.GetCell();
    std::putchar((int) output);
  }

  static OpWrite Create() {
    return OpWrite(M{});
  }
};

class OpJumpZero : public Op {
private:
  struct M {
    u32 offset;
    s32 direction;
  } m;

  explicit OpJumpZero(M m)
    : m(std::move(m))
  {}

public:
  virtual Instr Cmd() const override {
    return Instr::JUMP_ZERO;
  }

  virtual void Exec(MachineState &state) const override {
    if (state.GetCell() == 0) {
      if (m.direction > 0) {
        state.IncrementInstructionPointer(m.offset);
      } else {
        state.DecrementInstructionPointer(m.offset);
      }
    }
  }

  static OpJumpZero Create(u32 offset, s32 direction) {
    return OpJumpZero(M{
        .offset = offset,
        .direction = direction,
    });
  }
};

class OpJumpNonZero : public Op {
private:
  struct M {
    u32 offset;
    s32 direction;
  } m;

  explicit OpJumpNonZero(M m)
    : m(std::move(m))
  {}

public:
  virtual Instr Cmd() const override {
    return Instr::JUMP_NON_ZERO;
  }

  virtual void Exec(MachineState &state) const override {
    if (state.GetCell() != 0) {
      if (m.direction > 0) {
        state.IncrementInstructionPointer(m.offset);
      } else {
        state.DecrementInstructionPointer(m.offset);
      }
    }
  }

  static OpJumpNonZero Create(u32 offset, s32 direction) {
    return OpJumpNonZero(M{
        .offset = offset,
        .direction = direction,
    });
  }
};

#endif
