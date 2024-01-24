// SPDX-License-Identifier: MIT License
#ifndef BF_CC_OPTIMIZE_H
#define BF_CC_OPTIMIZE_H 1

#include "instr.h"


Instr *OptCommentLoop(Instr *);

Instr *OptFusionOp(Instr *);

Instr *OptPeep(Instr *);

class Optimizer final {
public:
  enum class Level {
    O0 = '0',
    O1 = '1',
    O2 = '2',
    O3 = '3',
  };

private:
  struct M {
    Level level;
  } m;

  explicit Optimizer(M m) : m(std::move(m)) {
  }

public:
  Instr *Run(Instr *) const noexcept;

  static Optimizer Create(Optimizer::Level level) noexcept {
    return Optimizer(M{
        .level = level,
    });
  }
};

class OptimizerPass {
private:
  struct M {
    Optimizer::Level level;
    const char *name;
    Instr* (*function)(Instr*);
  } m;

  explicit OptimizerPass(M m) : m(std::move(m)) {
  }

public:
  static OptimizerPass Create(const char *name, Instr* (*function)(Instr* ), Optimizer::Level level) {
    return OptimizerPass(M{.level = level, .name = name, .function = function});
  }

  Optimizer::Level Level() const {
    return m.level;
  }

  const char *Name() const {
    return m.name;
  }

  Instr* Run(Instr* instr) const {
    return m.function(instr);
  }
};

#endif /* BF_CC_OPTIMIZE_H */
