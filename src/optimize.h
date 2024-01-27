// SPDX-License-Identifier: MIT License
#ifndef BF_CC_OPTIMIZE_H
#define BF_CC_OPTIMIZE_H 1

#include <cstdio>

#include "instr.h"

void OptCommentLoop(Operation::Stream &);

void OptFusionOp(Operation::Stream &);

void OptPeep(Operation::Stream &);

void OptDelayPtr(Operation::Stream &);

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
  void Run(Operation::Stream &) const noexcept;

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
    void (*function)(Operation::Stream &);
  } m;

  explicit OptimizerPass(M m) : m(std::move(m)) {
  }

public:
  static OptimizerPass Create(const char *name, void (*function)(Operation::Stream &), Optimizer::Level level) {
    return OptimizerPass(M{.level = level, .name = name, .function = function});
  }

  Optimizer::Level Level() const {
    return m.level;
  }

  const char *Name() const {
    return m.name;
  }

  void Run(Operation::Stream &stream) const {
    m.function(stream);
  }
};

#endif /* BF_CC_OPTIMIZE_H */
