#ifndef BF_CC_MACHINE_H
#define BF_CC_MACHINE_H 1

#define DEFAULT_HEAP_SIZE 32768

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <utility>

class Heap {
private:
  struct M {
    int64_t data_pointer;
    uint8_t *data;
  } m;

  explicit Heap(M m) : m(std::move(m)) {}

public:
  static Heap Create(size_t size = DEFAULT_HEAP_SIZE) {
    // TODO: handle allocation failure
    return Heap(M{
        .data_pointer = 0,
        .data = (uint8_t *)calloc(size, sizeof(uint8_t)),
    });
  }

  ~Heap() { free(m.data); }

  inline void IncrementCell(uint8_t amount) {
    m.data[m.data_pointer] += amount;
  }

  inline void DecrementCell(uint8_t amount) {
    m.data[m.data_pointer] -= amount;
  }

  inline void SetCell(uint8_t value) {
    m.data[m.data_pointer] = value;
  }

  inline uint8_t GetCell() const {
    return m.data[m.data_pointer];
  }

  inline int64_t GetDataPointer() const { return m.data_pointer; }

  inline void IncrementDataPointer(int64_t amount) {
    m.data_pointer += amount;
  }

  inline void DecrementDataPointer(int64_t amount) {
    m.data_pointer -= amount;
  }

  inline void SetDataPointer(int64_t position) {
    m.data_pointer = position;
  }
};

#endif
