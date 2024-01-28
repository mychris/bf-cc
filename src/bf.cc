// SPDX-License-Identifier: MIT License
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "assembler.h"
#include "error.h"
#include "instr.h"
#include "interp.h"
#include "mem.h"
#include "optimize.h"
#include "parse.h"

enum class ExecMode {
  INTERPRETER = 'i',
  COMPILER = 'c',
};

static struct {
  const char *input_file_path = "";
  size_t heap_size = DEFAULT_HEAP_SIZE;
  ExecMode execution_mode = ExecMode::COMPILER;
  OptimizerLevel optimization_level = OptimizerLevel::O2;
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
    args.optimization_level = OptimizerLevel::O0;
    break;
  case '1':
    args.optimization_level = OptimizerLevel::O1;
    break;
  case '2':
    args.optimization_level = OptimizerLevel::O2;
    break;
  case '3':
    args.optimization_level = OptimizerLevel::O3;
    break;
  default:
    args.optimization_level = OptimizerLevel::O2;
    break;
  }
  if (0 == strlen(args.input_file_path)) {
    Error("No input file given");
  }
}

int main(int argc, char **argv) {
  parse_opts(argc, argv);
  char *content = Ensure(read_content(args.input_file_path));
  auto stream = Ensure(parse(content));
  stream.Prepend(Instruction::NOP);
  stream.Append(Instruction::NOP);
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
