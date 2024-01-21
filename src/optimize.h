#ifndef BF_CC_OPTIMIZE_H
#define BF_CC_OPTIMIZE_H 1

#include "instr.h"

class OptStage {
public:
  virtual Instr *Run(Instr *) = 0;
};

class OptCommentLoop : OptStage {
private:
  OptCommentLoop() {}

public:
  Instr *Run(Instr *);

  static OptCommentLoop Create() { return OptCommentLoop(); }
};

class OptFusionOp : OptStage {
private:
  OptFusionOp() {}

public:
  Instr *Run(Instr *);

  static OptFusionOp Create() { return OptFusionOp(); }
};

class OptPeep : OptStage {
private:
  OptPeep() {}

public:
  Instr *Run(Instr *);

  static OptPeep Create() { return OptPeep(); }

private:
  Instr *ReplaceSingleInstructionLoops(Instr *);
  Instr *ReplaceFindCellLoops(Instr *op);
  Instr *MergeSetIncrDecr(Instr *);
};

class Optimizer final {
private:
  Optimizer() {}

public:
  Instr *Run(Instr *);

  static Optimizer Create() { return Optimizer(); }
};

#endif /* BF_CC_OPTIMIZE_H */
