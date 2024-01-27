// SPDX-License-Identifier: MIT License
#ifndef BF_CC_HEAP_H
#define BF_CC_HEAP_H 1

#define DEFAULT_HEAP_SIZE 32768

#include <sys/mman.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
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

  explicit Heap(M m) noexcept : m(std::move(m)) {
  }

  Heap(const Heap &) = delete;
  Heap &operator=(const Heap &) = delete;

public:
  static std::variant<Heap, Err> Create(size_t size) noexcept;
  ~Heap();

  Heap(Heap &&other) noexcept : m(std::exchange(other.m, {0, 0, 0, nullptr})) {
  }

  Heap &operator=(Heap &&other) noexcept {
    std::swap(m, other.m);
    return *this;
  }

  inline void IncrementCell(uint8_t amount, int64_t offset = 0) noexcept {
    m.data[m.data_pointer + offset] += amount;
  }

  inline void DecrementCell(uint8_t amount, int64_t offset = 0) noexcept {
    m.data[m.data_pointer + offset] -= amount;
  }

  inline void SetCell(const uint8_t value, int64_t offset = 0) noexcept {
    m.data[m.data_pointer + offset] = value;
  }

  inline uint8_t GetCell(int64_t offset = 0) const noexcept {
    return m.data[m.data_pointer + offset];
  }

  inline void IncrementDataPointer(const int64_t amount) noexcept {
    m.data_pointer += amount;
  }

  inline void DecrementDataPointer(const int64_t amount) noexcept {
    m.data_pointer -= amount;
  }

  inline uint8_t *BaseAddress() noexcept {
    return m.data;
  }
};

#endif /* BF_CC_HEAP_H */
