#ifndef BF_CC_MACHINE_H
#define BF_CC_MACHINE_H 1

#include <cstdlib>
#include <utility>

#include "base.h"

class MachineState {
private:
  struct M {
    u32 data_pointer;
    u8 *data;
    u32 instruction_pointer;
  } m;

  explicit MachineState(M m)
    : m(std::move(m))
  {}

public:
  static MachineState Create() {
    // TODO: handle allocation failure
    return MachineState(M{
        .data_pointer = 0,
        .data = (u8 *)malloc(sizeof(u8) * 10000),
        .instruction_pointer = 0,
    });
  }

  ~MachineState() {
    free(m.data);
  }

  inline void IncrementCell(u8 amount) {
    m.data[m.data_pointer] += amount;
  }

  inline void DecrementCell(u8 amount) {
    m.data[m.data_pointer] -= amount;
  }

  inline void SetCell(u8 value) {
    m.data[m.data_pointer] = value;
  }

  inline u8 GetCell() const {
    return m.data[m.data_pointer];
  }

  inline void IncrementDataPointer(u32 amount) {
    m.data_pointer += amount;
  }

  inline void DecrementDataPointer(u32 amount) {
    m.data_pointer -= amount;
  }

  inline void SetDataPointer(u32 position) {
    m.data_pointer = position;
  }

  inline void IncrementInstructionPointer(u32 amount) {
    m.instruction_pointer += amount;
  }

  inline void DecrementInstructionPointer(u32 amount) {
    m.instruction_pointer -= amount;
  }

  inline void SetInstructionPointer(u32 position) {
    m.instruction_pointer = position;
  }

  inline u32 GetInstructionPointer() {
    return m.instruction_pointer;
  }
};

#endif
