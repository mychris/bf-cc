#ifndef BF_CC_OPTIMIZE_H
#define BF_CC_OPTIMIZE_H 1

#include "op.h"

class OptStage {
public:
  virtual Op* Run(Op*) = 0;
};

class OptCommentLoop : OptStage {
private:
  OptCommentLoop() {}

public:
  Op* Run(Op*);

  static OptCommentLoop Create() {
    return OptCommentLoop();
  }
};

class OptFusionOp : OptStage {
private:
  OptFusionOp() {}
public:
  Op* Run(Op*);

  static OptFusionOp Create() {
    return OptFusionOp();
  }
};

class OptPeep : OptStage {
private:
  OptPeep() {}

public:
  Op* Run(Op*);

  static OptPeep Create() {
    return OptPeep();
  }

private:
  Op* RemoveSetZero(Op*);
};

#endif
