#include "instr.h"
#include "optimize.h"

Instr* Optimizer::Run(Instr* i) {
  i = OptCommentLoop::Create().Run(i);
  i = OptFusionOp::Create().Run(i);
  i = OptPeep::Create().Run(i);
  return i;
}
