// SPDX-License-Identifier: MIT License
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

#include "assembler.h"
#include "error.h"
#include "fcntl.h"
#include "mem.h"
#include "instr.h"
#include "interp.h"
#include "optimize.h"
#include "unistd.h"

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

static struct {
  const char *input_file_path = "\0";
  size_t heap_size = DEFAULT_HEAP_SIZE;
  ExecMode execution_mode = ExecMode::COMPILER;
  Optimizer::Level optimization_level = Optimizer::Level::O2;
} args;

static void usage(void) {
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
  char opt_level = '2';
  const char *mem_size_string = NULL;
  while (argc--) {
    if (0 == strcmp("-h", argv[0]) || 0 == strcmp("--help", argv[0])) {
      usage();
      exit(0);
    } else if (0 == strncmp("-O", argv[0], 2)) {
      opt_level = argv[0][2];
      if (argv[0][2] == '\0' || argv[0][3] != '\0' || opt_level < '0' || opt_level > '3') {
        Error("Invalid optimization level: %s", argv[0]);
      }
    } else if (0 == strncmp("--optimize=", argv[0], 11)) {
      opt_level = argv[0][11];
      if (argv[0][11] == '\0' || argv[0][12] != '\0' || opt_level < '0' || opt_level > '3') {
        Error("Invalid optimization level: %s", argv[0]);
      }
    } else if (0 == strncmp("-m", argv[0], 2)) {
      mem_size_string = &argv[0][2];
    } else if (0 == strncmp("--memory=", argv[0], 9)) {
      mem_size_string = &argv[0][9];
    } else if (0 == strcmp("--interp", argv[0]) || 0 == strcmp("-i", argv[0])) {
      args.execution_mode = ExecMode::INTERPRETER;
    } else if (0 == strcmp("--comp", argv[0]) || 0 == strcmp("-c", argv[0])) {
      args.execution_mode = ExecMode::COMPILER;
    } else if (argv[0][0] != '-') {
      args.input_file_path = argv[0];
    } else if (0 == strcmp("-", argv[0])) {
      args.input_file_path = "/dev/stdin";
    } else {
      Error("Invalid argument: %s", argv[0]);
    }
    if (mem_size_string) {
      char *end = NULL;
      long long result = 0;
      errno = 0;
      result = std::strtoll(mem_size_string, &end, 0);
      if (result < 0 || errno == ERANGE || NULL == end || *end != '\0') {
        Error("Invalid heap memory size: %s", mem_size_string);
      }
      args.heap_size = (size_t) result;
    }
    argv++;
  }
  switch (opt_level) {
  case '0':
    args.optimization_level = Optimizer::Level::O0;
    break;
  case '1':
    args.optimization_level = Optimizer::Level::O1;
    break;
  case '2':
    args.optimization_level = Optimizer::Level::O2;
    break;
  case '3':
    args.optimization_level = Optimizer::Level::O3;
    break;
  default:
    args.optimization_level = Optimizer::Level::O2;
    break;
  }
  if (0 == strlen(args.input_file_path)) {
    Error("No input file given");
  }
}

std::variant<OperationStream, Err> parse(const char *input) {
  OperationStream stream = OperationStream::Create();
  stream.Prepend(Instruction::NOP);
  std::vector<Operation *> jump_stack = {};
  while (*input) {
    const char c = *input;
    switch (c) {
    case OP_INCR: {
      stream.Append(Instruction::INCR_CELL, 1);
    } break;
    case OP_DECR: {
      stream.Append(Instruction::DECR_CELL, 1);
    } break;
    case OP_NEXT: {
      stream.Append(Instruction::INCR_PTR, 1);
    } break;
    case OP_PREV: {
      stream.Append(Instruction::DECR_PTR, 1);
    } break;
    case OP_READ: {
      stream.Append(Instruction::READ, 0);
    } break;
    case OP_WRIT: {
      stream.Append(Instruction::WRITE, 0);
    } break;
    case OP_JMPF: {
      stream.Append(Instruction::JUMP_ZERO, 0);
      Operation *this_op = stream.Last();
      jump_stack.push_back(this_op);
    } break;
    case OP_JMPB: {
      Operation *other = jump_stack.back();
      jump_stack.pop_back();
      stream.Append(Instruction::JUMP_NON_ZERO, (Operation::operand_type) other);
      Operation *this_op = stream.Last();
      other->SetOperand1((Operation::operand_type) this_op);
    } break;
    default:
      break;
    }
    ++input;
  }
  stream.Append(Instruction::NOP);
  if (0 != jump_stack.size()) {
    return Err::UnmatchedJump();
  }
  return stream;
}

static std::variant<char *, Err> read_content(const char *filename) {
  char *content = NULL;
  off_t bytes_read = 0;
  const int fp = open(filename, O_RDONLY);
  if (0 > fp) {
    return Err::IO(errno);
  }
  const off_t fsize = lseek(fp, 0, SEEK_END);
  if (0 > fsize) {
    close(fp);
    return Err::IO(errno);
  }
  if (0 > lseek(fp, 0, SEEK_SET)) {
    close(fp);
    return Err::IO(errno);
  }
  content = (char *) calloc((size_t) fsize + 1, sizeof(char));
  while (bytes_read < fsize) {
    const ssize_t r = read(fp, content + bytes_read, (size_t) (fsize - bytes_read));
    if (0 > r && errno != EAGAIN) {
      free(content);
      close(fp);
      return Err::IO(errno);
    }
    bytes_read += r;
  }
  close(fp);
  return content;
}

int main(int argc, char **argv) {
  parse_opts(argc, argv);
  char *content = Ensure(read_content(args.input_file_path));
  auto stream = Ensure(parse(content));
  free(content);
  Optimizer::Create(args.optimization_level).Run(stream);
  auto heap = Ensure(Heap::Create(args.heap_size));
  switch (args.execution_mode) {
  case ExecMode::INTERPRETER: {
    auto interpreter = Interpreter::Create();
    interpreter.Run(heap, stream);
  } break;
  case ExecMode::COMPILER: {
    auto code_area = Ensure(CodeArea::Create());
    auto assembler = AssemblerX8664::Create(code_area);
    auto entry = Ensure(assembler.Assemble(stream));
    Ensure(code_area.MakeExecutable());
    entry(heap.BaseAddress());
  } break;
  }
  fflush(stdout);
  return 0;
}
