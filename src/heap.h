#ifndef BF_CC_MACHINE_H
#define BF_CC_MACHINE_H 1

#define DEFAULT_HEAP_SIZE 32768
#if defined(BF_HEAP_GUARD_PAGES)
#define GUARD_PAGES BF_HEAP_GUARD_PAGES
#else
#define GUARD_PAGES 2
#endif

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>

class Heap {
private:
  struct M {
    uint8_t *data;
    size_t page_size;
    size_t allocated;
    int64_t data_pointer;
  } m;

  explicit Heap(M m) : m(std::move(m)) {}

public:
  static Heap Create(size_t size = DEFAULT_HEAP_SIZE) {
    const size_t page_size = sysconf(_SC_PAGESIZE);
    // Round to size up to page_size
    size = ((size + page_size - 1) / page_size) * page_size;
    // Add two pages, which will be protected to guard for overflows
    size += page_size * (GUARD_PAGES * 2);
    uint8_t *mem = (uint8_t *)mmap(nullptr, size, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
      // TODO: report error
      assert(0);
    }
    if (0 != mprotect(mem, page_size * GUARD_PAGES, PROT_NONE)) {
      // TODO: report error
      assert(0);
    }
    if (0 != mprotect(mem + size - (page_size * GUARD_PAGES), page_size * GUARD_PAGES, PROT_NONE)) {
      // TODO: report error
      assert(0);
    }
    return Heap(M{
        .data = mem + (page_size * GUARD_PAGES),
        .page_size = page_size,
        .allocated = size,
        .data_pointer = 0,
    });
  }

  ~Heap() { munmap(m.data - (m.page_size * GUARD_PAGES), m.allocated); }

  inline void IncrementCell(uint8_t amount) {
    m.data[m.data_pointer] += amount;
  }

  inline void DecrementCell(uint8_t amount) {
    m.data[m.data_pointer] -= amount;
  }

  inline void SetCell(uint8_t value) { m.data[m.data_pointer] = value; }

  inline uint8_t GetCell() const { return m.data[m.data_pointer]; }

  inline int64_t GetDataPointer() const { return m.data_pointer; }

  inline void IncrementDataPointer(int64_t amount) { m.data_pointer += amount; }

  inline void DecrementDataPointer(int64_t amount) { m.data_pointer -= amount; }

  inline void SetDataPointer(int64_t position) { m.data_pointer = position; }

  inline uintptr_t BaseAddress() { return (uintptr_t)(m.data); }
};

#endif
