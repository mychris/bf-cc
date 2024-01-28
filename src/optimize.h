// SPDX-License-Identifier: MIT License
#ifndef BF_CC_OPTIMIZE_H
#define BF_CC_OPTIMIZE_H 1

#include <cstdio>

#include "instr.h"

void OptCommentLoop(OperationStream &);

void OptFusionOp(OperationStream &);

void OptPeep(OperationStream &);

void OptDelayPtr(OperationStream &);

void OptMultiplyLoop(OperationStream &);

enum class OptimizerLevel {
  O0 = '0',
  O1 = '1',
  O2 = '2',
  O3 = '3',
};

class Optimizer final {
public:
private:
  struct M {
    OptimizerLevel level;
  } m;

  explicit Optimizer(M m) : m(std::move(m)) {
  }

public:
  void Run(OperationStream &) const noexcept;

  static Optimizer Create(OptimizerLevel level) noexcept {
    return Optimizer(M{
        .level = level,
    });
  }
};

class OptimizerPass {
private:
  struct M {
    OptimizerLevel level;
    const char *name;
    void (*function)(OperationStream &);
  } m;

  explicit OptimizerPass(M m) : m(std::move(m)) {
  }

public:
  static OptimizerPass Create(const char *name, void (*function)(OperationStream &), OptimizerLevel level) {
    return OptimizerPass(M{.level = level, .name = name, .function = function});
  }

  OptimizerLevel Level() const {
    return m.level;
  }

  const char *Name() const {
    return m.name;
  }

  void Run(OperationStream &stream) const {
    m.function(stream);
  }
};

#endif /* BF_CC_OPTIMIZE_H */
