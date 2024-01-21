#include <cassert>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

#include <iterator>
#include <vector>

#include "assembler.h"
#include "instr.h"

CodeArea CodeArea::Create() {
  size_t page_size = sysconf(_SC_PAGESIZE);
  size_t reserved = 512 * 1024 * 1024;
  // TODO: make sure reserved is a multiple of page_size
  int flags = MAP_PRIVATE | MAP_ANONYMOUS;
  void *mem = mmap(nullptr, reserved, PROT_NONE, flags, -1, 0);
  if (mem == MAP_FAILED) {
    // TODO
    assert(0);
  }
  return CodeArea(M{
      .size = 0,
      .allocated = 0,
      .reserved = reserved,
      .page_size = page_size,
      .mem = (uint8_t *)mem,
  });
}

void CodeArea::Dump() {
  for (size_t i = 0; i < m.size; ++i) {
    if (i > 0 && i % 16 == 0) {
      printf("\n");
    } else if (i > 0 && i % 2 == 0) {
      printf(" ");
    }
    printf("%02X", m.mem[i]);
  }
  printf("\n");
}

CodeArea::~CodeArea() { munmap(m.mem, m.allocated); }

void CodeArea::EmitData(uint8_t *data, size_t length) {
  while (m.size + length >= m.allocated) {
    if (m.allocated == m.reserved) {
      // TODO
      assert(0);
    }
    int prot = PROT_READ | PROT_WRITE;
    int success = mprotect(m.mem + m.allocated, m.page_size, prot);
    if (success != 0) {
      // TODO
      assert(0);
    }
    m.allocated += m.page_size;
  }
  std::memcpy(m.mem + m.size, data, length);
  m.size += length;
  return;
}

void CodeArea::PatchData(uintptr_t p, uint8_t *data, size_t length) {
  std::memcpy((void *)p, data, length);
}

void CodeArea::MakeExecutable() {
  mprotect(m.mem, m.allocated, PROT_READ | PROT_EXEC);
}

static inline void EmitEntry(CodeArea &mem) {
  // Just to be save, push all callee saved registers
  // r12, r13, r14, r15, rbx, rsp, rbp
  mem.EmitCodeListing({
      0x41, 0x54, // PUSH r12
      0x41, 0x55, // PUSH r13
      0x41, 0x56, // PUSH r14
      0x41, 0x57, // PUSH r15
      0x53,       // PUSH rbx
      0x54,       // PUSH rsp
      0x55,       // PUSH rbp
  });
  // rdx is used to hold the pointer to the current cell
  // MOV rdx, rdi
  mem.EmitCodeListing({0x48, 0x89, 0xFA});
}

static inline void EmitExit(CodeArea &mem) {
  mem.EmitCodeListing({
      0x5D,       // POP rbp
      0x5C,       // POP rsp
      0x5B,       // POP rbx
      0x41, 0x5F, // POP r15
      0x41, 0x5E, // POP r14
      0x41, 0x5D, // POP r13
      0x41, 0x5C, // POP r12
  });
  // ret
  mem.EmitCodeListing({0xC3});
}

static inline void EmitNop(CodeArea &mem) {
  // Only used for debugging, so emit a bit more
  // NOPs
  mem.EmitCodeListing({0x90, 0x90, 0x90, 0x90});
}

static inline void EmitIncrCell(CodeArea &mem, uint8_t amount) {
  // ADD byte[rdx], amount
  mem.EmitCodeListing({0x80, 0x02, amount});
}

static inline void EmitDecrCell(CodeArea &mem, uint8_t amount) {
  // SUB byte[rdx], amount
  mem.EmitCodeListing({0x80, 0x2A, amount});
}

static inline void EmitSetCell(CodeArea &mem, uint8_t amount) {
  // MOV byte[rdx], amount
  mem.EmitCodeListing({0xC6, 0x02, amount});
}

static inline void EmitIncrPtr(CodeArea &mem, uintptr_t amount) {
  // ADD rdx, amount
  if (amount <= 127LLU) {
    mem.EmitCodeListing({0x48, 0x83, 0xC2, (uint8_t)amount});
  } else {
    assert(amount < UINT32_MAX);
    mem.EmitCodeListing({0x48, 0x81, 0xC2});
    mem.EmitCode((uint32_t)amount);
  }
}

static inline void EmitDecrPtr(CodeArea &mem, uintptr_t amount) {
  // SUB rdx, amount
  if (amount <= 127LLU) {
    mem.EmitCodeListing({0x48, 0x83, 0xEA, (uint8_t)amount});
  } else {
    assert(amount < UINT32_MAX);
    mem.EmitCodeListing({0x48, 0x81, 0xEA});
    mem.EmitCode((uint32_t)amount);
  }
}

static inline void EmitRead(CodeArea &mem) {
  // PUSH rdx
  mem.EmitCodeListing({0x52});
  mem.EmitCodeListing({// MOV rax, 0 (SYS_read)
                       0x48, 0xC7, 0xC0, 0x00, 0x00, 0x00, 0x00,
                       // MOV rdi, 0 (arg0: file descriptor)
                       0x48, 0xC7, 0xC7, 0x00, 0x00, 0x00, 0x00,
                       // MOV rsi, rdx (arg1: pointer)
                       0x48, 0x89, 0xD6,
                       // MOV rdx, 1 (arg2: count)
                       0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00,
                       // syscall
                       0x0F, 0x05});
  // POP rdx
  mem.EmitCodeListing({0x5A});
}

static inline void EmitWrite(CodeArea &mem) {
  // PUSH rdx
  mem.EmitCodeListing({0x52});
  mem.EmitCodeListing({// MOV rax, 1 (SYS_write)
                       0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00,
                       // MOV rdi, 1 (arg0: file descriptor)
                       0x48, 0xC7, 0xC7, 0x01, 0x00, 0x00, 0x00,
                       // MOV rsi, rdx (arg1: pointer)
                       0x48, 0x89, 0xD6,
                       // MOV rdx, 1 (arg2: count)
                       0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00,
                       // syscall
                       0x0F, 0x05});
  // POP rdx
  mem.EmitCodeListing({0x5A});
}

static inline void EmitJumpZero(CodeArea &mem) {
  // CMP byte[rdx], 0
  mem.EmitCodeListing({0x80, 0x3A, 0x00});
  // JZ
  mem.EmitCodeListing({0x0F, 0x84});
  // Jump will be patched later
  mem.EmitCodeListing({0x00, 0x00, 0x00, 0x00});
}

static inline void PatchJumpZero(CodeArea &mem, uintptr_t position,
                                 uintptr_t offset) {
  uint32_t offset32 = (uint32_t)offset;
  mem.PatchCode(position - 4, offset32);
}

static inline void EmitJumpNonZero(CodeArea &mem) {
  // CMP byte[rdx], 0
  mem.EmitCodeListing({0x80, 0x3A, 0x00});
  // JNZ
  mem.EmitCodeListing({0x0F, 0x85});
  // Jump will be patched later
  mem.EmitCodeListing({0x00, 0x00, 0x00, 0x00});
}

static inline void PatchJumpNonZero(CodeArea &mem, uintptr_t position,
                                    uintptr_t offset) {
  uint32_t offset32 = (uint32_t)offset;
  mem.PatchCode(position - 4, offset32);
}

static inline void EmitFindCellHigh(CodeArea &mem, uint8_t value,
                                    uintptr_t move_size) {
  assert(move_size < UINT32_MAX);
  mem.EmitCodeListing({// CMP byte[rdx], value
                       0x80, 0x3A, value,
                       // JE "to the end"
                       0x74, 0x09,
                       // ADD rdx, move_size
                       0x48, 0x81, 0xC2});
  mem.EmitCode((uint32_t)move_size);
  mem.EmitCodeListing({// JMP "back to SUB"
                       0xEB, 0xF2});
}

static inline void EmitFindCellLow(CodeArea &mem, uint8_t value,
                                   uintptr_t move_size) {
  assert(move_size < UINT32_MAX);
  mem.EmitCodeListing({// CMP byte[rdx], value
                       0x80, 0x3A, value,
                       // JE "to the end"
                       0x74, 0x09,
                       // SUB rdx, move_size
                       0x48, 0x81, 0xEA});
  mem.EmitCode((uint32_t)move_size);
  mem.EmitCodeListing({// JMP "back to SUB"
                       0xEB, 0xF2});
}

void AssemblerX8664::Assemble(Instr *code) {
  std::vector<std::pair<Instr *, uintptr_t>> jump_list = {};
  EmitEntry(m.mem);
  while (code) {
    switch (code->OpCode()) {
    case OpCode::NOP:
      EmitNop(m.mem);
      break;
    case OpCode::INCR_CELL:
      EmitIncrCell(m.mem, (uint8_t)code->Operand1());
      break;
    case OpCode::DECR_CELL:
      EmitDecrCell(m.mem, (uint8_t)code->Operand1());
      break;
    case OpCode::SET_CELL:
      EmitSetCell(m.mem, (uint8_t)code->Operand1());
      break;
    case OpCode::INCR_PTR:
      EmitIncrPtr(m.mem, (uintptr_t)code->Operand1());
      break;
    case OpCode::DECR_PTR:
      EmitDecrPtr(m.mem, (uintptr_t)code->Operand1());
      break;
    case OpCode::READ:
      EmitRead(m.mem);
      break;
    case OpCode::WRITE:
      EmitWrite(m.mem);
      break;
    case OpCode::JUMP_ZERO:
      EmitJumpZero(m.mem);
      jump_list.push_back({code, m.mem.CurrentWriteAddr()});
      break;
    case OpCode::JUMP_NON_ZERO:
      EmitJumpNonZero(m.mem);
      jump_list.push_back({code, m.mem.CurrentWriteAddr()});
      break;
    case OpCode::FIND_CELL_HIGH:
      EmitFindCellHigh(m.mem, (uint8_t)code->Operand1(), code->Operand2());
      break;
    case OpCode::FIND_CELL_LOW:
      EmitFindCellLow(m.mem, (uint8_t)code->Operand1(), code->Operand2());
      break;
    }
    code = code->Next();
  }
  // Patch the jumps
  for (const auto &[jump, code_pos] : jump_list) {
    uintptr_t target_pos = 0;
    for (const auto &[target_jump, pos2] : jump_list) {
      if (jump->Operand1() == (uintptr_t)target_jump) {
        assert(target_pos == 0 && "Multiple jumps to the same destination");
        target_pos = pos2;
      }
    }
    assert(target_pos > 0 && "Jump destination not found");
    if (jump->OpCode() == OpCode::JUMP_ZERO) {
      PatchJumpZero(m.mem, code_pos, target_pos - code_pos);
    } else if (jump->OpCode() == OpCode::JUMP_NON_ZERO) {
      PatchJumpNonZero(m.mem, code_pos, target_pos - code_pos);
    } else {
      assert(0 && "Invalid op code in jump list");
    }
  }
  EmitExit(m.mem);
}
