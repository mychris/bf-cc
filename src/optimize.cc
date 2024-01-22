// SPDX-License-Identifier: MIT License
#include "optimize.h"

#include "instr.h"

Instr *Optimizer::Run(Instr *i) const noexcept {
  if (m.level == OptLevel::O0) {
    return i;
  }
  // O1
  i = OptCommentLoop(i);
  i = OptFusionOp(i);
  if (m.level == OptLevel::O1) {
    return i;
  }
  // O2
  i = OptPeep(i);
  if (m.level == OptLevel::O2) {
    return i;
  }
  // O3
  return i;
}
