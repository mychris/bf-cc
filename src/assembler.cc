// SPDX-License-Identifier: MIT License
#include "assembler.h"

#include <sys/mman.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iterator>
#include <variant>
#include <vector>

#include "error.h"
#include "instr.h"

std::variant<CodeArea, Err> CodeArea::Create() noexcept {
  const size_t page_size = (size_t) sysconf(_SC_PAGESIZE);
  size_t reserved = 512 * 1024 * 1024;
  // Round reserved up to page_size
  reserved = ((reserved + page_size - 1) / page_size) * page_size;
  const int flags = MAP_PRIVATE | MAP_ANONYMOUS;
  void *mem = mmap(nullptr, reserved, PROT_NONE, flags, -1, 0);
  if (mem == MAP_FAILED) {
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
  }
}

Err CodeArea::EmitData(const uint8_t *data, const size_t length) {
  while (m.size + length >= m.allocated) {
    if (m.allocated == m.reserved) {
      return Err::CodeMmap(0);
    }
    const int prot = PROT_READ | PROT_WRITE;
    const int success = mprotect(m.mem + m.allocated, m.page_size, prot);
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
  if (0 != mprotect(m.mem, m.allocated, PROT_READ | PROT_EXEC)) {
    return Err::CodeMprotect(errno);
  }
  return Err::Ok();
}

static Err EmitEntry(CodeArea &mem) {
  // clang-format off
  // Just to be save, push all callee saved registers
  // r12, r13, r14, r15, rbx, rsp, rbp
  return mem.EmitCodeListing({
      0x41, 0x54,  // PUSH r12
      0x41, 0x55,  // PUSH r13
      0x41, 0x56,  // PUSH r14
      0x41, 0x57,  // PUSH r15
      0x53,        // PUSH rbx
      0x54,        // PUSH rsp
      0x55,        // PUSH rbp
      // rdx is used to hold the pointer to the current
      // cell MOV rdx, rdi
      0x48, 0x89, 0xFA
    });
  // clang-format on
}

static Err EmitExit(CodeArea &mem) {
  // clang-format off
  return mem.EmitCodeListing({
      0x5D,        // POP rbp
      0x5C,        // POP rsp
      0x5B,        // POP rbx
      0x41, 0x5F,  // POP r15
      0x41, 0x5E,  // POP r14
      0x41, 0x5D,  // POP r13
      0x41, 0x5C,  // POP r12
      // ret
      0xC3
    });
  // clang-format on
}

static Err EmitNop(CodeArea &mem) {
  // Only used for debugging, so emit a bit more
  // NOPs
  return mem.EmitCodeListing({0x90, 0x90, 0x90, 0x90});
}

static Err EmitIncrCell(CodeArea &mem, uint8_t amount) {
  // ADD byte[rdx], amount
  return mem.EmitCodeListing({0x80, 0x02, amount});
}

static Err EmitDecrCell(CodeArea &mem, uint8_t amount) {
  // SUB byte[rdx], amount
  return mem.EmitCodeListing({0x80, 0x2A, amount});
}

static Err EmitSetCell(CodeArea &mem, uint8_t amount) {
  // MOV byte[rdx], amount
  return mem.EmitCodeListing({0xC6, 0x02, amount});
}

static Err EmitIncrPtr(CodeArea &mem, uintptr_t amount) {
  // ADD rdx, amount
  if (amount <= 127LLU) {
    return mem.EmitCodeListing({0x48, 0x83, 0xC2, (uint8_t) amount});
  } else {
    assert(amount < UINT32_MAX);
    return mem.EmitCodeListing({0x48, 0x81, 0xC2}).and_then([&] { return mem.EmitCode((uint32_t) amount); });
  }
}

static Err EmitDecrPtr(CodeArea &mem, uintptr_t amount) {
  // SUB rdx, amount
  if (amount <= 127LLU) {
    return mem.EmitCodeListing({0x48, 0x83, 0xEA, (uint8_t) amount});
  } else {
    assert(amount < UINT32_MAX);
    return mem.EmitCodeListing({0x48, 0x81, 0xEA}).and_then([&] { return mem.EmitCode((uint32_t) amount); });
  }
}

static Err EmitRead(CodeArea &mem) {
  // clang-format off
  return mem.EmitCodeListing({
      // PUSH rdx
      0x52,
      // MOV rax, 0 (SYS_read)
      0x48, 0xC7, 0xC0, 0x00, 0x00, 0x00, 0x00,
      // MOV rdi, 0 (arg0: file descriptor)
      0x48, 0xC7, 0xC7, 0x00, 0x00, 0x00, 0x00,
      // MOV rsi, rdx (arg1: pointer)
      0x48, 0x89, 0xD6,
      // MOV rdx, 1 (arg2: count)
      0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00,
      // syscall
      0x0F, 0x05,
      // POP rdx
      0x5A,
      // CMP rax, 0x0
      0x48, 0x83, 0xF8, 0x00,
      // JNZ "next instruction"
      0x75, 0x03,
      // MOV byte[rdx], 0x0
      0xC6, 0x02, 0x00
    });
  // clang-format on
}

static Err EmitWrite(CodeArea &mem) {
  // clang-format off
  return mem.EmitCodeListing({
      // PUSH rdx
      0x52,
      // MOV rax, 1 (SYS_write)
      0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00,
      // MOV rdi, 1 (arg0: file descriptor)
      0x48, 0xC7, 0xC7, 0x01, 0x00, 0x00, 0x00,
      // MOV rsi, rdx (arg1: pointer)
      0x48, 0x89, 0xD6,
      // MOV rdx, 1 (arg2: count)
      0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00,
      // syscall
      0x0F, 0x05,
      // POP rdx
      0x5A
    });
  // clang-format on
}

static Err EmitJumpZero(CodeArea &mem) {
  // clang-format off
  return mem.EmitCodeListing({
      // CMP byte[rdx], 0
      0x80, 0x3A, 0x00,
      // JZ
      0x0F, 0x84,
      // Jump will be patched later
      0x00, 0x00, 0x00, 0x00
    });
  // clang-format on
}

static Err PatchJumpZero(CodeArea &mem, uint8_t *position, uintptr_t offset) {
  intptr_t signed_offset = (intptr_t) offset;
  if (signed_offset > (intptr_t) INT32_MAX || signed_offset < (intptr_t) INT32_MIN) {
    return Err::CodeInvalidOffset();
  }
  uint32_t offset32 = (uint32_t) offset;
  return mem.PatchCode(position - 4, offset32);
}

static Err EmitJumpNonZero(CodeArea &mem) {
  // clang-format off
  return mem.EmitCodeListing({
      // CMP byte[rdx], 0
      0x80, 0x3A, 0x00,
      // JNZ
      0x0F, 0x85,
      // Jump will be patched later
      0x00, 0x00, 0x00, 0x00
    });
  // clang-format on
}

static Err PatchJumpNonZero(CodeArea &mem, uint8_t *position, uintptr_t offset) {
  intptr_t signed_offset = (intptr_t) offset;
  if (signed_offset > (intptr_t) INT32_MAX || signed_offset < (intptr_t) INT32_MIN) {
    return Err::CodeInvalidOffset();
  }
  uint32_t offset32 = (uint32_t) offset;
  return mem.PatchCode(position - 4, offset32);
}

static Err EmitFindCellHigh(CodeArea &mem, uint8_t value, uintptr_t move_size) {
  if (move_size >= (uintptr_t) UINT32_MAX) {
    return Err::CodeInvalidOffset();
  }
  return mem
      .EmitCodeListing({// CMP byte[rdx], value
                        0x80,
                        0x3A,
                        value,
                        // JE "to the end"
                        0x74,
                        0x09,
                        // ADD rdx, move_size
                        0x48,
                        0x81,
                        0xC2})
      .and_then([&] { return mem.EmitCode((uint32_t) move_size); })
      .and_then([&] {
        // JMP "back to SUB"
        return mem.EmitCodeListing({0xEB, 0xF2});
      });
}

static Err EmitFindCellLow(CodeArea &mem, uint8_t value, uintptr_t move_size) {
  if (move_size >= (uintptr_t) UINT32_MAX) {
    return Err::CodeInvalidOffset();
  }
  return mem
      .EmitCodeListing({// CMP byte[rdx], value
                        0x80,
                        0x3A,
                        value,
                        // JE "to the end"
                        0x74,
                        0x09,
                        // SUB rdx, move_size
                        0x48,
                        0x81,
                        0xEA})
      .and_then([&] { return mem.EmitCode((uint32_t) move_size); })
      .and_then([&] {
        // JMP "back to SUB"
        return mem.EmitCodeListing({0xEB, 0xF2});
      });
}

std::variant<CodeEntry, Err> AssemblerX8664::Assemble(Instr *code) {
  std::vector<std::pair<Instr *, uint8_t *>> jump_list = {};
  Err err = Err::Ok();
  void *entry = m.mem.CurrentWriteAddr();
  err = EmitEntry(m.mem);
  if (!err.IsOk()) {
    return err;
  }
  while (code) {
    switch (code->OpCode()) {
    case Instr::Code::NOP:
      err = EmitNop(m.mem);
      break;
    case Instr::Code::INCR_CELL:
      err = EmitIncrCell(m.mem, (uint8_t) code->Operand1());
      break;
    case Instr::Code::DECR_CELL:
      err = EmitDecrCell(m.mem, (uint8_t) code->Operand1());
      break;
    case Instr::Code::SET_CELL:
      err = EmitSetCell(m.mem, (uint8_t) code->Operand1());
      break;
    case Instr::Code::INCR_PTR:
      err = EmitIncrPtr(m.mem, (uintptr_t) code->Operand1());
      break;
    case Instr::Code::DECR_PTR:
      err = EmitDecrPtr(m.mem, (uintptr_t) code->Operand1());
      break;
    case Instr::Code::READ:
      err = EmitRead(m.mem);
      break;
    case Instr::Code::WRITE:
      err = EmitWrite(m.mem);
      break;
    case Instr::Code::JUMP_ZERO:
      err = EmitJumpZero(m.mem);
      jump_list.push_back({code, m.mem.CurrentWriteAddr()});
      break;
    case Instr::Code::JUMP_NON_ZERO:
      err = EmitJumpNonZero(m.mem);
      jump_list.push_back({code, m.mem.CurrentWriteAddr()});
      break;
    case Instr::Code::FIND_CELL_HIGH:
      err = EmitFindCellHigh(m.mem, (uint8_t) code->Operand1(), code->Operand2());
      break;
    case Instr::Code::FIND_CELL_LOW:
      err = EmitFindCellLow(m.mem, (uint8_t) code->Operand1(), code->Operand2());
      break;
    }
    if (!err.IsOk()) {
      return err;
    }
    code = code->Next();
  }
  // Patch the jumps
  for (const auto &[jump, code_pos] : jump_list) {
    uint8_t *target_pos = 0;
    for (const auto &[target_jump, pos2] : jump_list) {
      if (jump->Operand1() == (uintptr_t) target_jump) {
        target_pos = pos2;
        break;
      }
    }
    assert(target_pos > (uint8_t *) 0 && "Jump destination not found");
    if (jump->OpCode() == Instr::Code::JUMP_ZERO) {
      err = PatchJumpZero(m.mem, code_pos, (uintptr_t) (target_pos - code_pos));
      if (!err.IsOk()) {
        return err;
      }
    } else if (jump->OpCode() == Instr::Code::JUMP_NON_ZERO) {
      err = PatchJumpNonZero(m.mem, code_pos, (uintptr_t) (target_pos - code_pos));
      if (!err.IsOk()) {
        return err;
      }
    } else {
      assert(0 && "Invalid op code in jump list");
    }
  }
  err = EmitExit(m.mem);
  if (!err.IsOk()) {
    return err;
  }
  return (CodeEntry) entry;
}
