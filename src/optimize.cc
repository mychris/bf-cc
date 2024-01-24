// SPDX-License-Identifier: MIT License
#include "optimize.h"

#include "instr.h"

Instr* Optimizer::Run(Instr* instr) const noexcept {
  static OptimizerPass pipeline[] = {
    OptimizerPass::Create("Remove comment loops", OptCommentLoop, Optimizer::Level::O1),
    OptimizerPass::Create("Fuse operators", OptFusionOp, Optimizer::Level::O1),
    OptimizerPass::Create("Peephole", OptPeep, Optimizer::Level::O2),
  };

  for (auto &stage : pipeline) {
    if (stage.Level() <= m.level) {
      instr = stage.Run(instr);
    }
  }
  return instr;
}
