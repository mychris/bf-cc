// SPDX-License-Identifier: MIT License
#include "mem.h"

#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <utility>
#include <variant>

#include "platform.h"

std::variant<Heap, Err> Heap::Create(size_t size) noexcept {
  const size_t page_size = Pagesize();
  if (size > (UINT32_MAX >> 1)) {
    return Err::OutOfMemory();
  }
  // Round size up to page_size
  size = ((size + page_size - 1) / page_size) * page_size;
  // Add guard pages in the front and the back
  size += page_size * (GUARD_PAGES * 2);
  uint8_t *mem = Ensure(Allocate(size));
  Ensure(Protect(mem + page_size * GUARD_PAGES, size - (page_size * GUARD_PAGES * 2), PROTECT_RW));
  return Heap(M{.page_size = page_size,
                .allocated = size,
                .available = size - (page_size * 2 * GUARD_PAGES),
                .data_pointer = 0,
                .data = mem + (page_size * GUARD_PAGES)});
}

Heap::~Heap() {
  if (m.data) {
    Deallocate(m.data - (m.page_size * GUARD_PAGES), m.allocated);
    m.data = nullptr;
  }
}

void Heap::Dump() const noexcept {
  uint8_t *ptr = m.data;
  size_t count = 0;
  const size_t available = m.available;
  while (count < available) {
    printf("%p  ", static_cast<void *>(ptr));
    for (int i = 0; count < available && i < 4; ++i) {
      printf("  ");
      for (int j = 0; count < available && j < 4; ++j) {
        printf(" %02X", *ptr);
        ++ptr;
        ++count;
      }
    }
    printf("\n");
  }
}

std::variant<CodeArea, Err> CodeArea::Create() noexcept {
  const size_t page_size = Pagesize();
  size_t reserved = 512 * 1024 * 1024;
  // Round reserved up to page_size
  reserved = ((reserved + page_size - 1) / page_size) * page_size;
  void *mem = Ensure(Allocate(reserved));
  return CodeArea(M{
      .size = page_size,  // keep the first page clean as guard page
      .allocated = 0,
      .reserved = reserved,
      .page_size = page_size,
      .mem = (uint8_t *) mem,
      .err = false,
  });
}

void CodeArea::Dump() const noexcept {
  for (size_t i = m.page_size; i < m.size; ++i) {
    if (i > 0 && i % 4 == 0) {
      printf("\n");
    }
    printf(" %02X", m.mem[i]);
  }
  printf("\n");
}

CodeArea::~CodeArea() {
  if (m.mem) {
    Deallocate(m.mem, m.allocated);
    m.mem = nullptr;
  }
}

void CodeArea::EmitData(const uint8_t *data, const size_t length) {
  if (!m.err) {
    while (m.size + length >= m.allocated) {
      if (m.allocated == m.reserved) {
        m.err = true;
      }
      Ensure(Protect(m.mem + m.allocated, m.page_size, PROTECT_RW));
      m.allocated += m.page_size;
    }
    std::memcpy(m.mem + m.size, data, length);
    m.size += length;
  }
}

void CodeArea::PatchData(uint8_t *p, const uint8_t *data, const size_t length) {
  std::memcpy(p, data, length);
}

Err CodeArea::MakeExecutable() {
  return Protect(m.mem, m.allocated, PROTECT_RX);
}
