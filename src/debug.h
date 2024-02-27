// SPDX-License-Identifier: MIT License
#ifndef BF_CC_DEBUG_H
#define BF_CC_DEBUG_H 1

#if defined(DEBUG) && DEBUG == 1
#define DEBUG_BUILD 1
#else
#undef DEBUG_BUILD
#endif

#if defined(DEBUG_BUILD)
#define ASSERT(test, format, ...)                                       \
  do {                                                                  \
    if (!(test)) {                                                      \
      Assert(__FILE__, __LINE__, (format) __VA_OPT__(,) __VA_ARGS__);   \
    }                                                                   \
  } while (0)
#else
#define ASSERT(test, format, ...)
#endif

#define GUARANTEE(test, format, ...)                                    \
  do {                                                                  \
    if (!(test)) {                                                      \
      Assert(__FILE__, __LINE__, (format) __VA_OPT__(,) __VA_ARGS__);   \
    }                                                                   \
  } while (0)

#define UNIMPLEMENTED() Assert(__FILE__, __LINE__, "%s: %s", __FUNCTION__, "unimplemented")

#define UNREACHABLE() Assert(__FILE__, __LINE__, "%s: %s", __FUNCTION__, "unreachable")

[[noreturn]] extern void Assert(const char*, unsigned long long, const char*, ...);

#endif /* BF_CC_DEBUG_H */
