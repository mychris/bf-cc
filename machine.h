#ifndef BF_CC_MACHINE_H
#define BF_CC_MACHINE_H 1

#include <cstdlib>
#include <utility>
#include <cassert>

#include "base.h"

#define MEMORY_SIZE 10000

class MachineState {
private:
  struct M {
    u32 data_pointer;
    u8 *data;
  } m;

  explicit MachineState(M m)
    : m(std::move(m))
  {}

public:
  static MachineState Create() {
    // TODO: handle allocation failure
    return MachineState(M{
        .data_pointer = 0,
        .data = (u8 *)calloc(MEMORY_SIZE, sizeof(u8)),
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
    assert(m.data_pointer < MEMORY_SIZE);
    m.data[m.data_pointer] = value;
  }

  inline u8 GetCell() const {
    assert(m.data_pointer < MEMORY_SIZE);
    return m.data[m.data_pointer];
  }

  inline u32 GetDataPointer() const {
    return m.data_pointer;
  }

  inline void IncrementDataPointer(u32 amount) {
    m.data_pointer += amount;
    assert(m.data_pointer < MEMORY_SIZE);
  }

  inline void DecrementDataPointer(u32 amount) {
    assert(m.data_pointer >= amount);
    m.data_pointer -= amount;
  }

  inline void SetDataPointer(u32 position) {
    assert(position < MEMORY_SIZE);
    m.data_pointer = position;
  }

};

#endif
