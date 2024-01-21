#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cstdlib>
#include "error.h"

const char* program_name = "bf-cc";

void Error(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "%s: ", program_name);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  if (fmt[strlen(fmt) - 1] != '\n') {
    fprintf(stderr, "\n");
  }
  exit(1);
}
