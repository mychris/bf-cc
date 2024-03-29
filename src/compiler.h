// SPDX-License-Identifier: MIT License
#ifndef BF_CC_COMPILER_H
#define BF_CC_COMPILER_H 1

#include <memory>
#include <utility>

#include "error.h"
#include "instr.h"
#include "mem.h"

class Compiler final {
private:
  using CodeEntry = void (*)(uint8_t *);

  struct M {
    CodeEntry entry;
    std::unique_ptr<CodeArea> mem;
  } m;

  explicit Compiler(M m) : m(std::move(m)) {
  }

  Compiler(const Compiler &) = delete;
  Compiler &operator=(const Compiler &) = delete;

public:
  static std::variant<Compiler, Err> Create() {
    auto mem = CodeArea::Create();
    if (0 != mem.index()) {
      return std::get<Err>(std::move(mem));
    }
    return Compiler(M{.entry = nullptr, .mem = std::make_unique<CodeArea>(std::move(std::get<CodeArea>(mem)))});
  }

  Compiler(Compiler &&other) noexcept : m(std::exchange(other.m, {nullptr, nullptr})) {
  }

  Compiler &operator=(Compiler &&other) noexcept {
    m = std::move(other.m);
    return *this;
  }

  Err Compile(OperationStream &, EOFMode) noexcept;

  void RunCode(Heap &heap) noexcept {
    CodeEntry entry = m.entry;
    uint8_t *heap_addr = heap.BaseAddress();
    entry(heap_addr);
  }

  void Dump() const noexcept {
    m.mem->Dump();
  }
};

#endif /* BF_CC_COMPILER_H */
