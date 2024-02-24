// SPDX-License-Identifier: MIT License
#include "mem.h"

#if defined(_WIN64)
#define IS_WINDOWS 1
#elif defined(__linux__)
#define IS_LINUX 1
#else
#error Unsupported plattform
#endif

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

#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <utility>
#include <variant>

static uint8_t *allocate(size_t size) {
#if defined(IS_WINDOWS)
  uint8_t *mem = (uint8_t *) VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (mem == nullptr) {
    errno = (int) GetLastError();
    return nullptr;
  }
  memset(mem, 0, size);
  DWORD old_protect = 0;
  if (!VirtualProtect(mem, size, PAGE_NOACCESS, &old_protect)) {
    errno = (int) GetLastError();
    return nullptr;
  }
  return mem;
#endif
#if defined(IS_LINUX)
  uint8_t *mem = (uint8_t *) mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) {
    return nullptr;
  }
  return mem;
#endif
}

static void deallocate(uint8_t *mem, size_t size) {
#if defined(IS_WINDOWS)
  VirtualFree(mem, size, MEM_DECOMMIT | MEM_RELEASE);
#endif
#if defined(IS_LINUX)
  munmap(mem, size);
#endif
}

static bool protect(uint8_t *mem, size_t size, unsigned int protect) {
#if defined(IS_WINDOWS)
  DWORD old_protect = 0;
  return VirtualProtect(mem, size, protect, &old_protect);
#endif
#if defined(IS_LINUX)
  return 0 == mprotect(mem, size, (int) protect);
#endif
}

static size_t pagesize() {
#if defined(IS_WINDOWS)
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return (size_t) si.dwPageSize;
#else
  return (size_t) sysconf(_SC_PAGESIZE);
#endif
}

std::variant<Heap, Err> Heap::Create(size_t size) noexcept {
  const size_t page_size = pagesize();
  if (size > (UINT32_MAX >> 1)) {
    return Err::HeapMmap(0);
  }
  // Round size up to page_size
  size = ((size + page_size - 1) / page_size) * page_size;
  // Add guard pages in the front and the back
  size += page_size * (GUARD_PAGES * 2);
  uint8_t *mem = allocate(size);
  if (mem == nullptr) {
    return Err::HeapMmap(errno);
  }
  if (!protect(mem + page_size * GUARD_PAGES, size - (page_size * GUARD_PAGES * 2), PROTECT_RW)) {
    return Err::HeapMprotect(errno);
  }
  return Heap(M{.page_size = page_size, .allocated = size, .data_pointer = 0, .data = mem + (page_size * GUARD_PAGES)});
}

Heap::~Heap() {
  if (m.data) {
    deallocate(m.data - (m.page_size * GUARD_PAGES), m.allocated);
    m.data = nullptr;
  }
}

std::variant<CodeArea, Err> CodeArea::Create() noexcept {
  const size_t page_size = pagesize();
  size_t reserved = 512 * 1024 * 1024;
  // Round reserved up to page_size
  reserved = ((reserved + page_size - 1) / page_size) * page_size;
  void *mem = allocate(reserved);
  if (mem == nullptr) {
    return Err::CodeMmap(errno);
  }
  return CodeArea(M{
      .size = page_size,  // keep the first page clean as guard page
      .allocated = 0,
      .reserved = reserved,
      .page_size = page_size,
      .mem = (uint8_t *) mem,
  });
}

void CodeArea::Dump() {
  for (size_t i = m.page_size; i < m.size; ++i) {
    if (i > 0 && i % 16 == 0) {
      printf("\n");
    } else if (i > 0 && i % 2 == 0) {
      printf(" ");
    }
    printf("%02X", m.mem[i]);
  }
  printf("\n");
}

CodeArea::~CodeArea() {
  if (m.mem) {
    deallocate(m.mem, m.allocated);
    m.mem = nullptr;
  }
}

Err CodeArea::EmitData(const uint8_t *data, const size_t length) {
  while (m.size + length >= m.allocated) {
    if (m.allocated == m.reserved) {
      return Err::CodeMmap(0);
    }
    if (!protect(m.mem + m.allocated, m.page_size, PROTECT_RW)) {
      return Err::CodeMprotect(errno);
    }
    m.allocated += m.page_size;
  }
  std::memcpy(m.mem + m.size, data, length);
  m.size += length;
  return Err::Ok();
}

Err CodeArea::PatchData(uint8_t *p, const uint8_t *data, const size_t length) {
  std::memcpy(p, data, length);
  return Err::Ok();
}

Err CodeArea::MakeExecutable() {
  if (!protect(m.mem, m.allocated, PROTECT_RX)) {
    return Err::CodeMprotect(errno);
  }
  return Err::Ok();
}
