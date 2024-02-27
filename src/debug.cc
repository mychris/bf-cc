// SPDX-License-Identifier: MIT License
#include "debug.h"

#include <stdarg.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

static std::vector<std::string> dump_flags;

[[noreturn]] extern void Assert(const char* file, unsigned long long line, const char* format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, "%s:%lld ", file, line);
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
  abort();
}

std::optional<std::string_view> IsDumpEnabled(std::string_view opt) {
  const size_t len = opt.size();
  for (const auto& flag : dump_flags) {
    if (flag.starts_with(opt)) {
      if (flag.size() == len) {
        return std::string_view{""};
      }
      if (flag[len] == '=') {
        std::string_view result{flag};
        return result.substr(len);
      }
      return {};
    }
  }
  return {};
}

void DumpEnable(std::string_view opt) {
  dump_flags.insert(dump_flags.begin(), std::string(opt));
}
