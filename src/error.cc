// SPDX-License-Identifier: MIT License
#include "error.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "platform.h"

const char *program_name = "bf-cc";

void Error(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::fprintf(stderr, "%s: ", program_name);
  std::vfprintf(stderr, fmt, ap);
  va_end(ap);
  if (fmt[strlen(fmt) - 1] != '\n') {
    std::fprintf(stderr, "\n");
  }
  std::exit(1);
}

void Error(const Err &error) {
  char native_err_str[256] = {};
  if (error.NativeErrno() != 0) {
    std::string err_string = NativeErrorToString(error.NativeErrno());
    sprintf(native_err_str, ": %s", err_string.c_str());
  }
  switch (error.Code()) {
  case Err::Code::UNMATCHED_JUMP:
    Error("Unmatched jump operation%s", native_err_str);
    break;
  case Err::Code::OUT_OF_MEMORY:
    Error("Out of memory%s", native_err_str);
    break;
  case Err::Code::MEM_ALLOCATE:
    Error("Out of memory%s", native_err_str);
    break;
  case Err::Code::MEM_PROTECT:
    Error("Failed to protect memory%s", native_err_str);
    break;
  case Err::Code::IO:
    Error("IO error%s", native_err_str);
    break;
  case Err::Code::CODE_INVALID_OFFSET:
    Error("Cannot emmit instruction - invalid offset");
  case Err::Code::OK:
    std::exit(1);
    break;
  }
}
