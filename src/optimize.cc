// SPDX-License-Identifier: MIT License
#include "optimize.h"

#include <cstdio>

#include "instr.h"

void Optimizer::Run(Operation::Stream &stream) const noexcept {
  static OptimizerPass pipeline[] = {
      OptimizerPass::Create("Remove comment loops", OptCommentLoop, Optimizer::Level::O1),
      OptimizerPass::Create("Fuse operators", OptFusionOp, Optimizer::Level::O1),
      OptimizerPass::Create("Peephole", OptPeep, Optimizer::Level::O2),
      OptimizerPass::Create("Delay Moves", OptDelayPtr, Optimizer::Level::O2),
  };

  for (auto &stage : pipeline) {
    if (stage.Level() <= m.level) {
      stage.Run(stream);
    }
  }
}
