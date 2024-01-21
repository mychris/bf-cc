#ifndef BF_CC_INTERP_H
#define BF_CC_INTERP_H 1

#include "instr.h"

class Interpreter final {
private:
  Interpreter() {}

public:
  void Run(Instr *);

  static Interpreter Create() { return Interpreter(); }
};

#endif
