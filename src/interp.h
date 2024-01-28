// SPDX-License-Identifier: MIT License
#ifndef BF_CC_INTERP_H
#define BF_CC_INTERP_H 1

#include "instr.h"
#include "mem.h"

class Interpreter final {
private:
  Interpreter() {
  }

public:
  static Interpreter Create() noexcept {
    return Interpreter();
  }

  void Run(Heap &, OperationStream &, EOFMode) const;
};

#endif /* BF_CC_INTERP_H */
