#include "heap.h"
#include <utility>
#include <variant>

#if defined(BF_HEAP_GUARD_PAGES)
#define GUARD_PAGES BF_HEAP_GUARD_PAGES
#else
#define GUARD_PAGES 0
#endif

std::variant<Heap, Err> Heap::Create(size_t size) noexcept {
  const size_t page_size = sysconf(_SC_PAGESIZE);
  if (size > (UINT32_MAX >> 1)) {
    return Err::HeapMmap(0);
  }
  // Round to size up to page_size
  size = ((size + page_size - 1) / page_size) * page_size;
  // Add guard pages in the front and the back
  size += page_size * (GUARD_PAGES * 2);
  uint8_t *mem = (uint8_t *)mmap(nullptr, size, PROT_READ | PROT_WRITE,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) {
    return Err::HeapMmap(errno);
  }
  if (0 != mprotect(mem, page_size * GUARD_PAGES, PROT_NONE)) {
    return Err::HeapMprotect(errno);
  }
  if (0 != mprotect(mem + (size - (page_size * GUARD_PAGES)),
                    page_size * GUARD_PAGES, PROT_NONE)) {
    return Err::HeapMprotect(errno);
  }
  return Heap(M{.page_size = page_size,
                .allocated = size,
                .data_pointer = 0,
                .data = mem + (page_size * GUARD_PAGES)});
}

Heap::~Heap() {
  if (m.data) {
    munmap(m.data - (m.page_size * GUARD_PAGES), m.allocated);
  }
}
