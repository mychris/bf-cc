// SPDX-License-Identifier: MIT License
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>

#include "compiler.h"
#include "debug.h"
#include "error.h"
#include "instr.h"
#include "interp.h"
#include "mem.h"
#include "optimize.h"
#include "parse.h"
#include "platform.h"

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
  fprintf(stderr, "  -c, --comp       Set the execution mode to: compiler\n");
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
  std::string_view dump_string{""};
  while (argc--) {
    std::string_view this_arg(argv[0]);
    if (this_arg == "-h" || this_arg == "--help") {
      usage();
      exit(0);
    } else if (this_arg.starts_with("-O")) {
      opt_level = this_arg[2];
      if (opt_level == '\0' || this_arg.size() > 3 || opt_level < '0' || opt_level > '3') {
        Error("Invalid optimization level: %s", this_arg.data());
      }
    } else if (this_arg.starts_with("--optimize=")) {
      opt_level = this_arg[11];
      if (opt_level == '\0' || this_arg.size() > 12 || opt_level < '0' || opt_level > '3') {
        Error("Invalid optimization level: %s", this_arg.data());
      }
    } else if (this_arg.starts_with("-m")) {
      mem_size_string = this_arg.substr(2);
    } else if (this_arg.starts_with("--memory=")) {
      mem_size_string = this_arg.substr(9);
    } else if (this_arg.starts_with("-d")) {
      dump_string = this_arg.substr(2);
    } else if (this_arg.starts_with("--dump=")) {
      dump_string = this_arg.substr(7);
    } else if (this_arg == "--interp" || this_arg == "-i") {
      args.execution_mode = ExecMode::INTERPRETER;
    } else if (this_arg == "--comp" || this_arg == "-c") {
      args.execution_mode = ExecMode::COMPILER;
    } else if (this_arg.starts_with("-e")) {
      eof_mode_string = this_arg.substr(2);
    } else if (this_arg.starts_with("--eof=")) {
      eof_mode_string = this_arg.substr(6);
    } else if (this_arg[0] != '-') {
      args.input_file_path = std::string(this_arg);
    } else if (this_arg == "-") {
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
        Error("Invalid heap memory size: %s", mem_size_string.data());
      }
      args.heap_size = (size_t) result;
      mem_size_string = std::string_view{""};
    }
    if (!dump_string.empty()) {
      const size_t length = dump_string.size();
      size_t start = 0;
      for (size_t i = 0; i < length; ++i) {
        if (dump_string[i] == ',') {
          if (i > start) {
            DumpEnable(dump_string.substr(start, i - start));
          }
          start = i + 1;
        }
      }
      if (length > start) {
        DumpEnable(dump_string.substr(start));
      }
      dump_string = std::string_view{""};
    }
    if (!eof_mode_string.empty()) {
      if ("keep" == eof_mode_string) {
        args.eof_mode = EOFMode::KEEP;
      } else if ("0" == eof_mode_string) {
        args.eof_mode = EOFMode::ZERO;
      } else if ("-1" == eof_mode_string) {
        args.eof_mode = EOFMode::NEG_ONE;
      } else {
        Error("Invalid EOF mode: %s", eof_mode_string.data());
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
  // Parse and optimize
  std::string raw_content = Ensure(ReadWholeFile(args.input_file_path));
  OperationStream stream = Ensure(Parse(raw_content));
  Optimizer::Create(args.optimization_level).Run(stream);
  if (IsDumpEnabled("prog")) {
    stream.Dump2();
  } else {
    // Allocate heap
    Heap heap = Ensure(Heap::Create(args.heap_size));
    // Compile and execute
    switch (args.execution_mode) {
    case ExecMode::INTERPRETER: {
      Interpreter interpreter = Interpreter::Create();
      interpreter.Run(heap, stream, args.eof_mode);
    } break;
    case ExecMode::COMPILER: {
      Compiler compiler = Ensure(Compiler::Create());
      Ensure(compiler.Compile(stream, args.eof_mode));
      if (IsDumpEnabled("code")) {
        compiler.Dump();
        return 0;
      } else {
        compiler.RunCode(heap);
      }
    } break;
    }
    if (IsDumpEnabled("heap")) {
      size_t from = 0;
      size_t to = 0;
      std::sscanf(IsDumpEnabled("heap").value().data(), "%zu-%zu", &from, &to);
      heap.Dump(from, to);
    }
  }
  return 0;
}
