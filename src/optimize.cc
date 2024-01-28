// SPDX-License-Identifier: MIT License
#include "optimize.h"

#include <cstdio>

#include "instr.h"

void Optimizer::Run(OperationStream &stream) const noexcept {
  static OptimizerPass pipeline[] = {
      OptimizerPass::Create("Remove comment loops", OptCommentLoop, OptimizerLevel::O1),
      OptimizerPass::Create("Fuse operators", OptFusionOp, OptimizerLevel::O1),
      OptimizerPass::Create("Peephole", OptPeep, OptimizerLevel::O2),
      OptimizerPass::Create("Delay Moves", OptDelayPtr, OptimizerLevel::O2),
      OptimizerPass::Create("Multilplcation Loops", OptMultiplyLoop, OptimizerLevel::O3),
  };

  for (auto &stage : pipeline) {
    if (stage.Level() <= m.level) {
      stage.Run(stream);
    }
  }
}
