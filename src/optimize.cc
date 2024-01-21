#include "optimize.h"
#include "instr.h"

OptLevel OL = OptLevel::O1;

Instr *Optimizer::Run(Instr *i) {
  if (OL == OptLevel::O0) {
    return i;
  }
  i = OptCommentLoop::Create().Run(i);
  i = OptFusionOp::Create().Run(i);
  if (OL == OptLevel::O1) {
    return i;
  }
  i = OptPeep::Create().Run(i);
  if (OL == OptLevel::O2) {
    return i;
  }
  return i;
}
