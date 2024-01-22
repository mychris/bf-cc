// SPDX-License-Identifier: MIT License
#ifndef BF_CC_ASSEMBLER_H
#define BF_CC_ASSEMBLER_H 1

#include <initializer_list>
#include <iterator>
#include <memory>
#include <variant>

#include "error.h"
#include "instr.h"

typedef void (*CodeEntry)(uint8_t *);

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
  CodeArea(CodeArea &&other) noexcept : m(std::exchange(other.m, {0, 0, 0, 0, nullptr})) {
  }

  ~CodeArea();
  static std::variant<CodeArea, Err> Create() noexcept;

  Err EmitCode(const uint32_t c) {
    return EmitData((const uint8_t *) &c, sizeof(uint32_t));
  }
  Err EmitCodeListing(std::initializer_list<uint8_t> l) {
    return EmitData((const uint8_t *) std::data(l), l.size());
  }

  Err PatchCode(uint8_t *p, const uint32_t c) {
    return PatchData(p, (const uint8_t *) &c, sizeof(uint32_t));
  }
  Err PatchCodeListing(uint8_t *p, std::initializer_list<uint8_t> l) {
    return PatchData(p, (const uint8_t *) std::data(l), l.size());
  }

  Err MakeExecutable();

  inline uint8_t *CurrentWriteAddr() {
    return m.mem + m.size;
  }

  void Dump();
};

class Assembler {};

class AssemblerX8664 {
private:
  struct M {
    CodeArea &mem;
  } m;

  explicit AssemblerX8664(M m) : m(std::move(m)) {
  }

public:
  static AssemblerX8664 Create(CodeArea &mem) {
    return AssemblerX8664(M{.mem = mem});
  }

  std::variant<CodeEntry, Err> Assemble(Instr *code);
};

#endif /* BF_CC_ASSEMBLER_H */
