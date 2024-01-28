// SPDX-License-Identifier: MIT License
#ifndef BF_CC_MEM_H
#define BF_CC_MEM_H 1

#define DEFAULT_HEAP_SIZE 32768

#if defined(BF_HEAP_GUARD_PAGES)
#define GUARD_PAGES BF_HEAP_GUARD_PAGES
#else
#define GUARD_PAGES 0
#endif

#include <cstdint>
#include <iterator>
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
    m = std::move(other.m);
    return *this;
  }

  inline void IncrementCell(uint8_t amount, int64_t offset) noexcept {
    m.data[m.data_pointer + offset] += amount;
  }

  inline void DecrementCell(uint8_t amount, int64_t offset) noexcept {
    m.data[m.data_pointer + offset] -= amount;
  }

  inline void SetCell(const uint8_t value, int64_t offset) noexcept {
    m.data[m.data_pointer + offset] = value;
  }

  inline uint8_t GetCell(int64_t offset) const noexcept {
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

  inline int64_t DataPointer() const noexcept {
    return m.data_pointer;
  }
};

class CodeArea final {
private:
  struct M {
    size_t size;
    size_t allocated;
    size_t reserved;
    size_t page_size;
    uint8_t *mem;
  } m;

  explicit CodeArea(M m) noexcept : m(std::move(m)) {
  }

  CodeArea(const CodeArea &) = delete;
  CodeArea &operator=(const CodeArea &) = delete;

  Err EmitData(const uint8_t *, const size_t);

  Err PatchData(uint8_t *, const uint8_t *, const size_t);

public:
  static std::variant<CodeArea, Err> Create() noexcept;

  ~CodeArea();

  CodeArea(CodeArea &&other) noexcept : m(std::exchange(other.m, {0, 0, 0, 0, nullptr})) {
  }

  CodeArea &operator=(CodeArea &&other) noexcept {
    m = std::move(other.m);
    return *this;
  }

  inline Err EmitCode(const uint32_t c) {
    return EmitData((const uint8_t *) &c, sizeof(uint32_t));
  }

  inline Err EmitCodeListing(std::initializer_list<uint8_t> l) {
    return EmitData((const uint8_t *) std::data(l), l.size());
  }

  inline Err PatchCode(uint8_t *p, const uint32_t c) {
    return PatchData(p, (const uint8_t *) &c, sizeof(uint32_t));
  }

  inline Err PatchCodeListing(uint8_t *p, std::initializer_list<uint8_t> l) {
    return PatchData(p, (const uint8_t *) std::data(l), l.size());
  }

  inline uint8_t *CurrentWriteAddr() {
    return m.mem + m.size;
  }

  Err MakeExecutable();

  void Dump();
};

#endif /* BF_CC_MEM_H */
