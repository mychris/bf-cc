#include <vector>

#include "op.h"
#include "optimize.h"

void OptFusionOp::Run(std::vector<Op>& ops) {
  m.ops = &ops;
  m.fuse_list.clear();
  FuseOperations();
  FixupJumps();
}

void OptFusionOp::FuseOperations() {
  size_t idx = 0;
  u32 fuse_count = 0;
  while (idx < m.ops->size()) {
    Instr idx_cmd = (*m.ops)[idx].Cmd();
    if (idx_cmd == Instr::INCR_CELL
        || idx_cmd == Instr::DECR_CELL
        || idx_cmd == Instr::INCR_PTR
        || idx_cmd == Instr::DECR_PTR) {
      size_t end = idx;
      s32 amount = 0;
      while ((*m.ops)[end].Cmd() == idx_cmd) {
        amount += (*m.ops)[end].Operand1();
        ++end;
      }
      if (end > idx + 1) {
        m.ops->erase(std::next(m.ops->begin(), idx + 1), std::next(m.ops->begin(), end));
        (*m.ops)[idx] = Op::Create(idx_cmd, amount);
        fuse_count += end - idx - 1;
        m.fuse_list.push_back({idx + fuse_count, fuse_count});
      }
    }
    ++idx;
  }
}

void OptFusionOp::FixupJumps() {
  // TODO: optimize this!
  for (size_t idx = 0; idx < m.ops->size(); ++idx) {
    if ((*m.ops)[idx].IsJump()) {
      u32 target = (*m.ops)[idx].Operand1();
      u32 correct = 0;
      for (const auto& [idx, fuse_count] : m.fuse_list) {
        if (idx >= target) {
          break;
        }
        correct = fuse_count;
      }
      (*m.ops)[idx] = Op::Create((*m.ops)[idx].Cmd(), (*m.ops)[idx].Operand1() - correct);
    }
  }
}
