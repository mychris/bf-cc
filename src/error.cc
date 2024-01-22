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
  case ErrorCode::OUT_OF_MEMORY:
    Error("Out of memory%s", native_err_str);
  case ErrorCode::HEAP_MMAP:
    Error("Out of heap memory%s", native_err_str);
  case ErrorCode::HEAP_MPROTECT:
    Error("Failed to protect heap guard pages%s", native_err_str);
  case ErrorCode::CODE_MMAP:
    Error("Out of code memory%s", native_err_str);
  case ErrorCode::CODE_MPROTECT:
    Error("Failed to protect code area%s", native_err_str);
  case ErrorCode::IO:
    Error("IO error: %s", native_err_str);
  case ErrorCode::OK:
    std::exit(1);
    break;
  }
}
