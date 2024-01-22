// SPDX-License-Identifier: MIT License
#include <csignal>
#ifndef BF_CC_OPTIMIZE_H
#define BF_CC_OPTIMIZE_H 1

#include "instr.h"

enum class OptLevel {
  O0 = '0',
  O1 = '1',
  O2 = '2',
  O3 = '3',
};

Instr *OptCommentLoop(Instr *);

Instr *OptFusionOp(Instr *);

Instr *OptPeep(Instr *);

class Optimizer final {
private:
  struct M {
    OptLevel level;
  } m;

  explicit Optimizer(M m) : m(std::move(m)) {
  }

public:
  Instr *Run(Instr *) const noexcept;

  static Optimizer Create(OptLevel level) noexcept {
    return Optimizer(M{
        .level = level,
    });
  }
};

#endif /* BF_CC_OPTIMIZE_H */
