#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <memory>
#include <utility>
#include <vector>

#include "base.h"
#include "machine.h"
#include "instr.h"
#include "optimize.h"
#include "interp.h"

#define OP_INCR '+'
#define OP_DECR '-'
#define OP_NEXT '>'
#define OP_PREV '<'
#define OP_READ ','
#define OP_WRIT '.'
#define OP_JMPF '['
#define OP_JMPB ']'

Instr *parse(char *input) {
  Instr *head = Instr::Allocate(OpCode::NOP);
  Instr *tail = head;
  std::vector<Instr *> jump_stack = {};
  while (*input) {
    const char c = *input;
    Instr *this_op = nullptr;
    switch (c) {
    case OP_INCR: {
      this_op = Instr::Allocate(OpCode::INCR_CELL, 1);
    } break;
    case OP_DECR: {
      this_op = Instr::Allocate(OpCode::DECR_CELL, 1);
    } break;
    case OP_NEXT: {
      this_op = Instr::Allocate(OpCode::INCR_PTR, 1);
    } break;
    case OP_PREV: {
      this_op = Instr::Allocate(OpCode::DECR_PTR, 1);
    } break;
    case OP_READ: {
      this_op = Instr::Allocate(OpCode::READ, 0);
    } break;
    case OP_WRIT: {
      this_op = Instr::Allocate(OpCode::WRITE, 0);
    } break;
    case OP_JMPF: {
      this_op = Instr::Allocate(OpCode::JUMP_ZERO, 0);
      jump_stack.push_back(this_op);
    } break;
    case OP_JMPB: {
      Instr *other = jump_stack.back();
      jump_stack.pop_back();
      this_op = Instr::Allocate(OpCode::JUMP_NON_ZERO, (uintptr_t)other);
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
    fcontent = (char *)calloc(fsize + 1, sizeof(char));
    fread(fcontent, 1, fsize, fp);
    fclose(fp);
  }
  return fcontent;
}

int main(int argc, char **argv) {
  auto machine = MachineState::Create();
  auto optimizer = Optimizer::Create();
  auto interpreter = Interpreter::Create();
  char *path = argv[1];
  char *content = readcontent(path);
  auto op = parse(content);
  free(content);
  op = optimizer.Run(op);
  interpreter.Run(op);
  while (op) {
    Instr* next = op->Next();
    delete op;
    op = next;
  }
  return 0;
}