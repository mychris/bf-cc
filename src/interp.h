// SPDX-License-Identifier: MIT License
#ifndef BF_CC_INTERP_H
#define BF_CC_INTERP_H 1

#include "mem.h"
#include "instr.h"

class Interpreter final {
private:
  Interpreter() {
  }

public:
  void Run(Heap &, Operation::Stream &);

  static Interpreter Create() {
    return Interpreter();
  }
};

#endif /* BF_CC_INTERP_H */
