#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <utility>
#include <vector>
#include <memory>

#include "base.h"
#include "machine.h"
#include "op.h"

#define OP_INCR '+'
#define OP_DECR '-'
#define OP_NEXT '>'
#define OP_PREV '<'
#define OP_READ ','
#define OP_WRIT '.'
#define OP_JMPF '['
#define OP_JMPB ']'

std::vector<Op> parse2(char* input) {
  std::vector<Op> operations = {};
  std::vector<u32> jump_stack = {};
  while (*input) {
    const char c = *input;
    switch (c) {
    case OP_INCR: {
      operations.push_back(Op::Create(Instr::INCR_CELL, 1));
    } break;
    case OP_DECR: {
      operations.push_back(Op::Create(Instr::DECR_CELL, 1));
    } break;
    case OP_NEXT: {
      operations.push_back(Op::Create(Instr::INCR_PTR, 1));
    } break;
    case OP_PREV: {
      operations.push_back(Op::Create(Instr::DECR_PTR, 1));
    } break;
    case OP_READ: {
      operations.push_back(Op::Create(Instr::READ, 0));
    } break;
    case OP_WRIT: {
      operations.push_back(Op::Create(Instr::WRITE, 0));
    } break;
    case OP_JMPF: {
      jump_stack.push_back(operations.size());
      operations.push_back(Op::Create(Instr::JUMP_ZERO, 0, +1));
    } break;
    case OP_JMPB: {
      u32 other = jump_stack.back();
      u32 amount = operations.size() - other;
      jump_stack.pop_back();
      operations.push_back(Op::Create(Instr::JUMP_NON_ZERO, amount, -1));
      operations[other] = Op::Create(Instr::JUMP_ZERO, amount, +1);
    } break;
    default:
      break;
    }
    ++input;
  }
  return operations;
}

static char *readcontent(const char *filename)
{
    char *fcontent = NULL;
    int fsize = 0;
    FILE *fp;
    fp = fopen(filename, "r");
    if(fp) {
        fseek(fp, 0, SEEK_END);
        fsize = ftell(fp);
        rewind(fp);
        fcontent = (char*) malloc(sizeof(char) * fsize);
        fread(fcontent, 1, fsize, fp);
        fclose(fp);
    }
    return fcontent;
}

int main(int argc, char **argv)
{
  char* path = argv[1];
  char* content = readcontent(path);
  auto operations = parse2(content);
  auto machine = MachineState::Create();
  while (machine.GetInstructionPointer() < operations.size()) {
    const auto& op = operations[machine.GetInstructionPointer()];
    machine.IncrementInstructionPointer(1);
    op.Exec(machine);
  }
  return 0;
}
