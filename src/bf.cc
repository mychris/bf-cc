#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <memory>
#include <utility>
#include <vector>

#include "error.h"
#include "heap.h"
#include "assembler.h"
#include "instr.h"
#include "interp.h"
#include "optimize.h"

#define OP_INCR '+'
#define OP_DECR '-'
#define OP_NEXT '>'
#define OP_PREV '<'
#define OP_READ ','
#define OP_WRIT '.'
#define OP_JMPF '['
#define OP_JMPB ']'

enum class ExecMode {
  INTERPRETER = 'i',
  COMPILER = 'c',
};

static const char *input_file_path = "\0";
static size_t heap_size = DEFAULT_HEAP_SIZE;
static ExecMode execution_mode = ExecMode::COMPILER;

static void usage() {
  fprintf(stderr, "Usage: %s [-h] [-O(0|1|2|3)] [-mMEMORY_SIZE] [(-i|-c)] PROGRAM\n", program_name);
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -O, --optimize=  Set the optimization level to 0, 1, 2, or 3\n");
  fprintf(stderr, "  -m, --memory=    Set the heap memory size\n");
  fprintf(stderr, "  -i, --interp     Set the execution mode to: interpreter\n");
  fprintf(stderr, "  -c, --compiler   Set the execution mode to: compiler\n");
  fprintf(stderr, "  -h, --help       Display this help message\n");
}

static void parse_opts(int argc, char **argv) {
  program_name = argv[0];
  argc--;
  argv++;
  char opt_level = '1';
  const char *mem_size_string = NULL;
  while (argc--) {
    if (0 == strcmp("-h", argv[0])
        || 0 == strcmp("--help", argv[0])) {
      usage();
      exit(0);
    } else if (0 == strncmp("-O", argv[0], 2)) {
      opt_level = argv[0][2];
      if (argv[0][2] == '\0'
          || argv[0][3] != '\0'
          || opt_level < '0'
          || opt_level > '3') {
        Error("Invalid optimization level: %s", argv[0]);
      }
    } else if (0 == strncmp("--optimize=", argv[0], 11)) {
      opt_level = argv[0][11];
      if (argv[0][11] == '\0'
          || argv[0][12] != '\0'
          || opt_level < '0'
          || opt_level > '3') {
        Error("Invalid optimization level: %s", argv[0]);
      }
    } else if (0 == strncmp("-m", argv[0], 2)) {
      mem_size_string = &argv[0][2];
    } else if (0 == strncmp("--memory=", argv[0], 9)) {
      mem_size_string = &argv[0][9];
    } else if (0 == strcmp("--interp", argv[0])
               || 0 == strcmp("-i", argv[0])) {
      execution_mode = ExecMode::INTERPRETER;
    } else if (0 == strcmp("--comp", argv[0])
               || 0 == strcmp("-c", argv[0])) {
      execution_mode = ExecMode::COMPILER;
    } else if (argv[0][0] != '-') {
      input_file_path = argv[0];
    } else {
      Error("Invalid argument: %s", argv[0]);
    }
    if (mem_size_string) {
      char* end = NULL;
      long long result = 0;
      errno = 0;
      result = std::strtoll(mem_size_string, &end, 0);
      if (result < 0
          || errno == ERANGE
          || NULL == end
          || *end != '\0') {
        Error("Invalid heap memory size: %s", mem_size_string);
      }
      heap_size = result;
    }
    argv++;
  }
  if (0 == strlen(input_file_path)) {
    Error("No input file given");
  }
}

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
  tail->SetNext(Instr::Allocate(OpCode::NOP));
  return head;
}

static std::variant<char*, Err> read_content(const char *filename) {
  char *fcontent = NULL;
  int fsize = 0;
  FILE *fp;
  fp = fopen(filename, "r");
  if (!fp) {
    return Err::IO(errno);
  }
  fseek(fp, 0, SEEK_END);
  fsize = ftell(fp);
  rewind(fp);
  fcontent = (char *)calloc(fsize + 1, sizeof(char));
  fread(fcontent, 1, fsize, fp);
  fclose(fp);
  return fcontent;
}

int main(int argc, char **argv) {
  parse_opts(argc, argv);
  char *content = std::get<char*>(read_content(input_file_path));
  auto op = parse(content);
  free(content);
  auto optimizer = Optimizer::Create();
  op = optimizer.Run(op);
  switch (execution_mode) {
  case ExecMode::INTERPRETER: {
    auto heap = Heap::Create(heap_size);
    auto interpreter = Interpreter::Create();
    interpreter.Run(std::get<Heap>(heap), op);
  } break;
  case ExecMode::COMPILER: {
    auto code_area = CodeArea::Create();
    auto assembler = AssemblerX8664::Create(std::get<CodeArea>(code_area));
    auto entry = (code_entry) std::get<CodeArea>(code_area).BaseAddress();
    auto heap = Ensure(Heap::Create(heap_size));
    Ensure(assembler.Assemble(op));
    Ensure(std::get<CodeArea>(code_area).MakeExecutable());
    entry(heap.BaseAddress());
  } break;
  }
  fflush(stdout);
  while (op) {
    Instr *next = op->Next();
    delete op;
    op = next;
  }
  return 0;
}
