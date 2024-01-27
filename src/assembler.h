// SPDX-License-Identifier: MIT License
#ifndef BF_CC_ASSEMBLER_H
#define BF_CC_ASSEMBLER_H 1

#include <initializer_list>
#include <memory>
#include <variant>

#include "error.h"
#include "instr.h"
#include "mem.h"

typedef void (*CodeEntry)(uint8_t *);

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

  std::variant<CodeEntry, Err> Assemble(OperationStream &);
};

#endif /* BF_CC_ASSEMBLER_H */
