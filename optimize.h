#ifndef BF_CC_OPTIMIZE_H
#define BF_CC_OPTIMIZE_H 1

#include <vector>

#include "op.h"

class OptStage {
public:
  virtual void Run(std::vector<Op>&) = 0;
};

class OptFusionOp : OptStage {
private:
  struct M {
    std::vector<Op>* ops;
    std::vector<std::pair<u32, u32>> fuse_list;
  } m;

  explicit OptFusionOp(M m)
    : m(std::move(m))
  {}

public:
  void Run(std::vector<Op> &);

  static OptFusionOp Create() {
    return OptFusionOp(M{
        .ops = nullptr,
        .fuse_list = {},
    });
  }

private:
  void FuseOperations();
  void FixupJumps();
};

#endif
