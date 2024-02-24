// SPDX-License-Identifier: MIT License
#ifndef BF_CC_PLATFORM_H
#define BF_CC_PLATFORM_H 1

#if defined(_WIN64)
#define IS_WINDOWS 1
#elif defined(__linux__)
#define IS_LINUX 1
#else
#error Unsupported plattform
#endif

#include <cstdint>
#include <string>

#include "error.h"

#if defined(IS_WINDOWS)
#include <windows.h>
#define PROTECT_NONE (PAGE_NOACCESS)
#define PROTECT_RW (PAGE_READWRITE)
#define PROTECT_RX (PAGE_EXECUTE_READ)
#endif

#if defined(IS_LINUX)
#include <sys/mman.h>
#define PROTECT_NONE (PROT_NONE)
#define PROTECT_RW (PROT_READ | PROT_WRITE)
#define PROTECT_RX (PROT_READ | PROT_EXEC)
#endif

extern std::string NativeErrorToString(int64_t native_error);

extern std::variant<uint8_t *, Err> Allocate(size_t);

extern void Deallocate(uint8_t *, size_t);

extern Err Protect(uint8_t *, size_t, unsigned int);

extern size_t Pagesize();

extern std::variant<std::string, Err> ReadWholeFile(const std::string_view);

#endif /* BF_CC_PLATFORM_H */
