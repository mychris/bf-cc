#ifndef BF_CC_MACHINE_H
#define BF_CC_MACHINE_H 1

#define MEMORY_SIZE 10000

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <utility>

class MachineState {
private:
  struct M {
    int64_t data_pointer;
    uint8_t *data;
  } m;

  explicit MachineState(M m) : m(std::move(m)) {}

public:
  static MachineState Create() {
    // TODO: handle allocation failure
    return MachineState(M{
        .data_pointer = 0,
        .data = (uint8_t *)calloc(MEMORY_SIZE, sizeof(uint8_t)),
    });
  }

  ~MachineState() { free(m.data); }

  inline void IncrementCell(uint8_t amount) {
    m.data[m.data_pointer] += amount;
  }

  inline void DecrementCell(uint8_t amount) {
    m.data[m.data_pointer] -= amount;
  }

  inline void SetCell(uint8_t value) {
    assert(m.data_pointer < MEMORY_SIZE);
    m.data[m.data_pointer] = value;
  }

  inline uint8_t GetCell() const {
    assert(m.data_pointer < MEMORY_SIZE);
    return m.data[m.data_pointer];
  }

  inline int64_t GetDataPointer() const { return m.data_pointer; }

  inline void IncrementDataPointer(int64_t amount) {
    m.data_pointer += amount;
    assert(m.data_pointer < MEMORY_SIZE);
  }

  inline void DecrementDataPointer(int64_t amount) {
    assert(m.data_pointer >= amount);
    m.data_pointer -= amount;
  }

  inline void SetDataPointer(int64_t position) {
    assert(position < MEMORY_SIZE);
    m.data_pointer = position;
  }
};

#endif
