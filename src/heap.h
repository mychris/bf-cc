#ifndef BF_CC_HEAP_H
#define BF_CC_HEAP_H 1

#define DEFAULT_HEAP_SIZE 32768

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>
#include <variant>

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

  Heap(const Heap &) = delete;
  Heap &operator=(const Heap &) = delete;

public:
  static std::variant<Heap, Err> Create(size_t size) noexcept;
  ~Heap();

  Heap(Heap &&other) noexcept : m(std::exchange(other.m, {0, 0, 0, nullptr})) {}

  Heap &operator=(Heap &&other) noexcept {
    std::swap(m, other.m);
    return *this;
  }

  inline void IncrementCell(uint8_t amount) {
    m.data[m.data_pointer] += amount;
  }

  inline void DecrementCell(uint8_t amount) {
    m.data[m.data_pointer] -= amount;
  }

  inline void SetCell(const uint8_t value) noexcept {
    m.data[m.data_pointer] = value;
  }

  inline uint8_t GetCell() const noexcept { return m.data[m.data_pointer]; }

  inline void IncrementDataPointer(const int64_t amount) noexcept {
    m.data_pointer += amount;
  }

  inline void DecrementDataPointer(const int64_t amount) noexcept {
    m.data_pointer -= amount;
  }

  inline uint8_t *BaseAddress() noexcept { return m.data; }
};

#endif /* BF_CC_HEAP_H */
