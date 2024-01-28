// SPDX-License-Identifier: MIT License
#include "optimize.h"

#include "instr.h"

void Optimizer::Run(OperationStream &stream) const noexcept {
  static const OptimizerPass pipeline[] = {
      OptimizerPass::Create("Remove comment loops", OptCommentLoop, OptimizerLevel::O1),
      OptimizerPass::Create("Fuse operators", OptFusionOp, OptimizerLevel::O1),
      OptimizerPass::Create("Peephole", OptPeep, OptimizerLevel::O2),
      OptimizerPass::Create("Delay Moves", OptDelayPtr, OptimizerLevel::O2),
      OptimizerPass::Create("Multiplicative Loops", OptMultiplyLoop, OptimizerLevel::O3),
  };

  for (const auto &stage : pipeline) {
    if (stage.Level() <= m.level) {
      stage.Run(stream);
    }
  }
}
