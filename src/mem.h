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
#include <utility>
#include <variant>

#include "debug.h"
#include "error.h"

class Heap final {
private:
  struct M {
    size_t page_size;
    size_t allocated;
    size_t available;
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

  Heap(Heap &&other) noexcept : m(std::exchange(other.m, {0, 0, 0, 0, nullptr})) {
  }

  Heap &operator=(Heap &&other) noexcept {
    m = std::move(other.m);
    return *this;
  }

  inline void IncrementCell(uint8_t amount, int64_t offset) noexcept {
    ASSERT(m.data_pointer + offset >= 0, "cell outside of memory area");
    ASSERT(m.data_pointer + offset < static_cast<int64_t>(m.available), "cell outside of memory area");
    m.data[m.data_pointer + offset] += amount;
  }

  inline void DecrementCell(uint8_t amount, int64_t offset) noexcept {
    ASSERT(m.data_pointer + offset >= 0, "cell outside of memory area");
    ASSERT(m.data_pointer + offset < static_cast<int64_t>(m.available), "cell outside of memory area");
    m.data[m.data_pointer + offset] -= amount;
  }

  inline void SetCell(const uint8_t value, int64_t offset) noexcept {
    ASSERT(m.data_pointer + offset >= 0, "cell outside of memory area");
    ASSERT(m.data_pointer + offset < static_cast<int64_t>(m.available), "cell outside of memory area");
    m.data[m.data_pointer + offset] = value;
  }

  inline uint8_t GetCell(int64_t offset) const noexcept {
    ASSERT(m.data_pointer + offset >= 0, "cell outside of memory area");
    ASSERT(m.data_pointer + offset < static_cast<int64_t>(m.available), "cell outside of memory area");
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

  void Dump(size_t, size_t) const noexcept;
};

class CodeArea final {
private:
  struct M {
    size_t size;
    size_t allocated;
    size_t reserved;
    size_t page_size;
    uint8_t *mem;
    bool err;
  } m;

  explicit CodeArea(M m) noexcept : m(std::move(m)) {
  }

  CodeArea(const CodeArea &) = delete;
  CodeArea &operator=(const CodeArea &) = delete;

  void EmitData(const uint8_t *, const size_t);

  void PatchData(uint8_t *, const uint8_t *, const size_t);

public:
  static std::variant<CodeArea, Err> Create() noexcept;

  ~CodeArea();

  CodeArea(CodeArea &&other) noexcept : m(std::exchange(other.m, {0, 0, 0, 0, nullptr, true})) {
  }

  CodeArea &operator=(CodeArea &&other) noexcept {
    m = std::move(other.m);
    return *this;
  }

  inline void EmitCode(const uint32_t c) {
    return EmitData((const uint8_t *) &c, sizeof(uint32_t));
  }

  inline void EmitCode64(const uint64_t c) {
    return EmitData((const uint8_t *) &c, sizeof(uint64_t));
  }

  inline void EmitCodeListing(std::initializer_list<uint8_t> l) {
    return EmitData((const uint8_t *) std::data(l), l.size());
  }

  inline void PatchCode(uint8_t *p, const uint32_t c) {
    return PatchData(p, (const uint8_t *) &c, sizeof(uint32_t));
  }

  inline void PatchCodeListing(uint8_t *p, std::initializer_list<uint8_t> l) {
    return PatchData(p, (const uint8_t *) std::data(l), l.size());
  }

  inline uint8_t *CurrentWriteAddr() {
    return m.mem + m.size;
  }

  Err MakeExecutable();

  bool HasWriteError() const noexcept {
    return m.err;
  };

  void Dump() const noexcept;
};

#endif /* BF_CC_MEM_H */
