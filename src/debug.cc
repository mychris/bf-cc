// SPDX-License-Identifier: MIT License
#include "debug.h"

#include <stdarg.h>

#include <cstdio>
#include <cstdlib>

[[noreturn]] extern void Assert(const char* file, unsigned long long line, const char* format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, "%s:%lld ", file, line);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
  abort();
}
