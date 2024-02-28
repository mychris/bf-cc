// SPDX-License-Identifier: MIT License
#include "mem.h"

#include <unistd.h>

#include <cmath>
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
  auto alloc_result = Allocate(size);
  if (alloc_result.index() != 0) {
    return std::get<Err>(alloc_result);
  }
  uint8_t *mem = std::get<uint8_t *>(alloc_result);
  Err err = Protect(mem + page_size * GUARD_PAGES, size - (page_size * GUARD_PAGES * 2), PROTECT_RW);
  if (!err.IsOk()) {
    return err;
  }
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

void Heap::Dump(size_t from, size_t to) const noexcept {
  const size_t row_count = 16;
  if (to <= from) {
    return;
  }
  to = std::min(to, m.available);
  from = std::min(from, m.available);
  to = (to + row_count) / row_count * row_count;
  from = from / row_count * row_count;
  printf("Heap dump for cells %zu to %zu\n", from, to - 1);
  uint8_t *ptr = m.data;
  ptr += from;
  size_t count = from;
  const size_t available = m.available;
  const int digits = static_cast<int>(std::ceil(std::log10(static_cast<double>(available))));
  while (count < available && count < to) {
    printf("%0*zu  ", digits, ptr - m.data);
    for (size_t i = 0; count < available && i < row_count; ++i) {
      if (i % 4 == 0) {
        printf("  ");
      }
      printf(" %02X", *ptr);
      ++ptr;
      ++count;
    }
    printf("\n");
  }
}

std::variant<CodeArea, Err> CodeArea::Create() noexcept {
  const size_t page_size = Pagesize();
  size_t reserved = 512 * 1024 * 1024;
  // Round reserved up to page_size
  reserved = ((reserved + page_size - 1) / page_size) * page_size;
  auto alloc_result = Allocate(reserved);
  if (alloc_result.index() != 0) {
    return std::get<Err>(alloc_result);
  }
  return CodeArea(M{
      .size = page_size,  // keep the first page clean as guard page
      .allocated = 0,
      .reserved = reserved,
      .page_size = page_size,
      .mem = std::get<uint8_t *>(alloc_result),
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
