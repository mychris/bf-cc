// SPDX-License-Identifier: MIT License
#include "mem.h"

#include <sys/mman.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <utility>
#include <variant>

static void* allocate(size_t size) {
  uint8_t *mem = (uint8_t *) mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) {
    return nullptr;
  }
  return mem;
}

static void deallocate(uint8_t *ptr, size_t size) {
  munmap(ptr, size);
}

static int commit(uint8_t *ptr, size_t size) {
  return mprotect(ptr, size, PROT_READ | PROT_WRITE);
}

static int executable(uint8_t *ptr, size_t size) {
  return mprotect(ptr, size, PROT_READ | PROT_EXEC);
}

std::variant<Heap, Err> Heap::Create(size_t size) noexcept {
  const size_t page_size = (size_t) sysconf(_SC_PAGESIZE);
  if (size > (UINT32_MAX >> 1)) {
    return Err::HeapMmap(0);
  }
  // Round size up to page_size
  size = ((size + page_size - 1) / page_size) * page_size;
  // Add guard pages in the front and the back
  size += page_size * (GUARD_PAGES * 2);
  uint8_t *mem = (uint8_t *) allocate(size);
  if (mem == nullptr) {
    return Err::HeapMmap(errno);
  }
  if (0 != commit(mem + GUARD_PAGES, size - (GUARD_PAGES * 2))) {
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
  const size_t page_size = (size_t) sysconf(_SC_PAGESIZE);
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
    munmap(m.mem, m.allocated);
    m.mem = nullptr;
  }
}

Err CodeArea::EmitData(const uint8_t *data, const size_t length) {
  while (m.size + length >= m.allocated) {
    if (m.allocated == m.reserved) {
      return Err::CodeMmap(0);
    }
    const int success = commit(m.mem + m.allocated, m.page_size);
    if (success != 0) {
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
  if (0 != executable(m.mem, m.allocated)) {
    return Err::CodeMprotect(errno);
  }
  return Err::Ok();
}
