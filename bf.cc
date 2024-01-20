#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <memory>
#include <utility>
#include <vector>

#include "base.h"
#include "machine.h"
#include "op.h"
#include "optimize.h"

#define OP_INCR '+'
#define OP_DECR '-'
#define OP_NEXT '>'
#define OP_PREV '<'
#define OP_READ ','
#define OP_WRIT '.'
#define OP_JMPF '['
#define OP_JMPB ']'

Op *parse(char *input) {
  Op *head = Op::Allocate(Instr::NOP);
  Op *tail = head;
  std::vector<Op *> jump_stack = {};
  while (*input) {
    const char c = *input;
    Op *this_op = nullptr;
    switch (c) {
    case OP_INCR: {
      this_op = Op::Allocate(Instr::INCR_CELL, 1);
    } break;
    case OP_DECR: {
      this_op = Op::Allocate(Instr::DECR_CELL, 1);
    } break;
    case OP_NEXT: {
      this_op = Op::Allocate(Instr::INCR_PTR, 1);
    } break;
    case OP_PREV: {
      this_op = Op::Allocate(Instr::DECR_PTR, 1);
    } break;
    case OP_READ: {
      this_op = Op::Allocate(Instr::READ, 0);
    } break;
    case OP_WRIT: {
      this_op = Op::Allocate(Instr::WRITE, 0);
    } break;
    case OP_JMPF: {
      this_op = Op::Allocate(Instr::JUMP_ZERO, 0);
      jump_stack.push_back(this_op);
    } break;
    case OP_JMPB: {
      Op *other = jump_stack.back();
      jump_stack.pop_back();
      this_op = Op::Allocate(Instr::JUMP_NON_ZERO, (uintptr_t)other);
      other->SetOperand1((uintptr_t)this_op);
    } break;
    default:
      break;
    }
    if (this_op) {
      tail->SetNext(this_op);
      tail = this_op;
    }
    ++input;
  }
  return head;
}

static char *readcontent(const char *filename) {
  char *fcontent = NULL;
  int fsize = 0;
  FILE *fp;
  fp = fopen(filename, "r");
  if (fp) {
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    rewind(fp);
    fcontent = (char *)malloc(sizeof(char) * fsize);
    fread(fcontent, 1, fsize, fp);
    fclose(fp);
  }
  return fcontent;
}

int main(int argc, char **argv) {
  auto machine = MachineState::Create();
  char *path = argv[1];
  char *content = readcontent(path);
  auto op = parse(content);
  {
    OptCommentLoop optim0 = OptCommentLoop::Create();
    op = optim0.Run(op);
    OptFusionOp optim1 = OptFusionOp::Create();
    op = optim1.Run(op);
    OptPeep optim2 = OptPeep::Create();
    op = optim2.Run(op);
  }
  while (op) {
    op = op->Exec(machine);
  }
  return 0;
}
