// SPDX-License-Identifier: MIT License
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "platform.h"
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
  std::string input_file_path{""};
  size_t heap_size = DEFAULT_HEAP_SIZE;
  ExecMode execution_mode = ExecMode::COMPILER;
  OptimizerLevel optimization_level = OptimizerLevel::O2;
  EOFMode eof_mode = EOFMode::KEEP;
} args;

static void usage(void) {
  fprintf(stderr, "Usage: %s [-h] [-O(0|1|2|3)] [-mMEMORY_SIZE] [(-i|-c)] [-e(keep|0|-1)]PROGRAM\n", program_name);
  fprintf(stderr, "\n");
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -O, --optimize=  Set the optimization level to 0, 1, 2, or 3\n");
  fprintf(stderr, "  -m, --memory=    Set the heap memory size\n");
  fprintf(stderr, "  -i, --interp     Set the execution mode to: interpreter\n");
  fprintf(stderr, "  -c, --compiler   Set the execution mode to: compiler\n");
  fprintf(stderr, "  -e, --eof=       Set EOF to 'keep', '0', or '-1'\n");
  fprintf(stderr, "  -h, --help       Display this help message\n");
}

static void parse_opts(int argc, char **argv) {
  program_name = argv[0];
  argc--;
  argv++;
  char opt_level{'2'};
  std::string_view mem_size_string{""};
  std::string_view eof_mode_string{""};
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
      mem_size_string = std::string_view{&argv[0][2]};
    } else if (0 == strncmp("--memory=", argv[0], 9)) {
      mem_size_string = std::string_view{&argv[0][9]};
    } else if (0 == strcmp("--interp", argv[0]) || 0 == strcmp("-i", argv[0])) {
      args.execution_mode = ExecMode::INTERPRETER;
    } else if (0 == strcmp("--comp", argv[0]) || 0 == strcmp("-c", argv[0])) {
      args.execution_mode = ExecMode::COMPILER;
    } else if (0 == strncmp("-e", argv[0], 2)) {
      eof_mode_string = std::string_view{&argv[0][2]};
    } else if (0 == strncmp("--eof=", argv[0], 6)) {
      eof_mode_string = std::string_view{&argv[0][6]};
    } else if (argv[0][0] != '-') {
      args.input_file_path = std::string{argv[0]};
    } else if (0 == strcmp("-", argv[0])) {
      args.input_file_path = std::string{"/dev/stdin"};
    } else {
      Error("Invalid argument: %s", argv[0]);
    }
    if (!mem_size_string.empty()) {
      char *end = NULL;
      long long result = 0;
      errno = 0;
      result = std::strtoll(mem_size_string.data(), &end, 0);
      if (result < 0 || errno == ERANGE || NULL == end || *end != '\0') {
        Error("Invalid heap memory size: %s", mem_size_string);
      }
      args.heap_size = (size_t) result;
      mem_size_string = std::string_view{""};
    }
    if (!eof_mode_string.empty()) {
      if ("keep" == eof_mode_string) {
        args.eof_mode = EOFMode::KEEP;
      } else if ("0" == eof_mode_string) {
        args.eof_mode = EOFMode::ZERO;
      } else if ("-1" == eof_mode_string) {
        args.eof_mode = EOFMode::NEG_ONE;
      } else {
        Error("Invalid EOF mode: %s", eof_mode_string);
      }
      eof_mode_string = std::string_view{""};
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
  if (args.input_file_path.empty()) {
    Error("No input file given");
  }
}

int main(int argc, char **argv) {
  parse_opts(argc, argv);
  auto raw_content = Ensure(ReadWholeFile(args.input_file_path));
  OperationStream stream = Ensure(Parse(raw_content));
  Optimizer::Create(args.optimization_level).Run(stream);
  Heap heap = Ensure(Heap::Create(args.heap_size));
  switch (args.execution_mode) {
  case ExecMode::INTERPRETER: {
    Interpreter interpreter = Interpreter::Create();
    interpreter.Run(heap, stream, args.eof_mode);
  } break;
  case ExecMode::COMPILER: {
    CodeArea code_area = Ensure(CodeArea::Create());
    AssemblerX8664 assembler = AssemblerX8664::Create(code_area);
    CodeEntry entry = Ensure(assembler.Assemble(stream, args.eof_mode));
    Ensure(code_area.MakeExecutable());
    entry(heap.BaseAddress());
  } break;
  }
  fflush(stdout);
  return 0;
}
