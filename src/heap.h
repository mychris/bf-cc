#ifndef BF_CC_HEAP_H
#define BF_CC_HEAP_H 1

#define DEFAULT_HEAP_SIZE 32768

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>
#include <variant>
#include <memory>

#include "error.h"

class Heap final {
private:
  struct M {
    size_t page_size;
    size_t allocated;
    int64_t data_pointer;
    uint8_t *data;
  } m;

  explicit Heap(M m) noexcept : m(std::move(m)) {}

  Heap(const Heap&) = delete;
  Heap& operator=(const Heap&) = delete;

public:
  Heap(Heap&& other)
    : m(std::exchange(other.m, {0, 0, 0, nullptr}))
  {}

  ~Heap();
  static std::variant<Heap, Err> Create(size_t size) noexcept;

  inline void IncrementCell(uint8_t amount) { m.data[m.data_pointer] += amount; }

  inline void DecrementCell(uint8_t amount) { m.data[m.data_pointer] -= amount; }

  inline void SetCell(uint8_t value) { m.data[m.data_pointer] = value; }

  inline uint8_t GetCell() const { return m.data[m.data_pointer]; }

  inline void IncrementDataPointer(int64_t amount) { m.data_pointer += amount; }

  inline void DecrementDataPointer(int64_t amount) { m.data_pointer -= amount; }

  inline uintptr_t BaseAddress() { return (uintptr_t)(m.data); }
};

#endif /* BF_CC_HEAP_H */
