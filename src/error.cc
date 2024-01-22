#include "error.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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
  char native_err_str[128] = {};
  if (error.NativeErrno() != 0) {
    sprintf(native_err_str, ": %s", strerror(error.NativeErrno()));
  }
  switch (error.Code()) {
  case Err::Code::OUT_OF_MEMORY:
    Error("Out of memory%s", native_err_str);
    break;
  case Err::Code::HEAP_MMAP:
    Error("Out of heap memory%s", native_err_str);
    break;
  case Err::Code::HEAP_MPROTECT:
    Error("Failed to protect heap guard pages%s", native_err_str);
    break;
  case Err::Code::CODE_MMAP:
    Error("Out of code memory%s", native_err_str);
    break;
  case Err::Code::CODE_MPROTECT:
    Error("Failed to protect code area%s", native_err_str);
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
