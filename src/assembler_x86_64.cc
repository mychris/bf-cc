// SPDX-License-Identifier: MIT License
#include <cassert>
#include <cstdio>
#include <variant>
#include <vector>

#include "assembler.h"
#include "error.h"
#include "instr.h"
#include "platform.h"

static void do_write(uint8_t *c) {
  std::putchar((int) *c);
}

static void do_read(uint8_t *c, uint32_t mode) {
  int input = std::getchar();
  if (EOF == input) {
    switch (static_cast<EOFMode>(mode)) {
    case EOFMode::KEEP: {
      input = static_cast<int>(*c);
    } break;
    case EOFMode::ZERO: {
      input = 0;
    } break;
    case EOFMode::NEG_ONE: {
      input = -1;
    } break;
    }
  }
  *c = static_cast<uint8_t>(input);
}

Err EmitEntry(CodeArea &mem) {
  // clang-format off
  // Just to be save, push ALL registers
  return mem.EmitCodeListing({
      0x41, 0x57,  // PUSH r15
      0x41, 0x56,  // PUSH r14
      0x41, 0x55,  // PUSH r13
      0x41, 0x54,  // PUSH r12
      0x41, 0x53,  // PUSH r11
      0x41, 0x52,  // PUSH r10
      0x41, 0x51,  // PUSH r9
      0x41, 0x50,  // PUSH r8
      0x57,        // PUSH rdi
      0x56,        // PUSH rsi
      0x55,        // PUSH rbp
      0x54,        // PUSH rsp
      0x53,        // PUSH rbx
      0x52,        // PUSH rdx
      0x51,        // PUSH rcx
      0x50,        // PUSH rax
      // rdx is used to hold the pointer to the current cell
#if defined(IS_WINDOWS)
      // MOV rdx, rcx
      0x48, 0x89, 0xCA
#endif
#if defined(IS_LINUX)
      // MOV rdx, rdi
      0x48, 0x89, 0xFA
#endif
    });
  // clang-format on
}

Err EmitExit(CodeArea &mem) {
  // clang-format off
  return mem.EmitCodeListing({
      0x58,        // POP rax
      0x59,        // POP rcx
      0x5A,        // POP rdx
      0x5B,        // POP rbx
      0x5C,        // POP rsp
      0x5D,        // POP rbp
      0x5E,        // POP rsi
      0x5F,        // POP rdi
      0x41, 0x58,  // POP r8
      0x41, 0x59,  // POP r9
      0x41, 0x5A,  // POP r10
      0x41, 0x5B,  // POP r11
      0x41, 0x5C,  // POP r12
      0x41, 0x5D,  // POP r13
      0x41, 0x5E,  // POP r14
      0x41, 0x5F,  // POP r15
      0xC3         // ret
    });
  // clang-format on
}

Err EmitNop(CodeArea &mem) {
  // Only used for debugging, so emit a bit more NOPs
  return mem.EmitCodeListing({0x90, 0x90, 0x90, 0x90});
}

Err EmitIncrCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (0 == offset) {
    // ADD byte[rdx], amount
    return mem.EmitCodeListing({0x80, 0x02, amount});
  } else if (128 > offset && -128 < offset) {
    // ADD byte[rdx+offset], amount
    return mem.EmitCodeListing({0x80, 0x42, (uint8_t) offset, (uint8_t) amount});
  } else {
    // clang-format off
    // ADD byte[rdx+offset], amount
    return mem.EmitCodeListing({0x80, 0x82})
      .and_then([&] {
        return mem.EmitCode((uint32_t) offset);
      }).and_then([&] {
        return mem.EmitCodeListing({amount});
      });
    // clang-format on
  }
}

Err EmitDecrCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (offset == 0) {
    // SUB byte[rdx], amount
    return mem.EmitCodeListing({0x80, 0x2A, amount});
  } else if (128 > offset && -128 < offset) {
    // SUB byte[rdx+offset], amount
    return mem.EmitCodeListing({0x80, 0x6A, (uint8_t) offset, (uint8_t) amount});
  } else {
    // clang-format off
    // SUB byte[rdx+offset], amount
    return mem.EmitCodeListing({0x80, 0xAA})
      .and_then([&] {
        return mem.EmitCode((uint32_t) offset);
      })
      .and_then([&] {
        return mem.EmitCodeListing({amount});
      });
    // clang-format on
  }
}

Err EmitImullCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (0 == offset) {
    // This should never be created
    assert(0 != offset);
    return mem.EmitCodeListing({0x90});
  }
  Err e = Err::Ok();
  // clang-format off
  if (1 == amount) {
    // MOV al, byte[rdx]
    e = mem.EmitCodeListing({0x8A, 0x02});
  } else {
    // rdx needs to be saved, sinc MUL overwrites it
    e = Err::Ok().and_then([&] {
      // MOV r8, rdx
      return mem.EmitCodeListing({0x49, 0x89, 0xD0});
    }).and_then([&] {
      // MOV rax, amount
      return mem.EmitCodeListing({0x48, 0xC7, 0xC0}).and_then([&] { return mem.EmitCode((uint32_t) amount); });
    }).and_then([&] {
      // MOV bl, byte[rdx]
      return mem.EmitCodeListing({0x8A, 0x1A});
    }).and_then([&] {
      // MUL rbx
      return mem.EmitCodeListing({0x48, 0xF7, 0xE3});
    }).and_then([&] {
      // MOV rdx, r8
      return mem.EmitCodeListing({0x4C, 0x89, 0xC2});
    });
  }
  if (!e.IsOk()) {
    return e;
  }
  if (128 > offset && -128 < offset) {
    // ADD byte[rdx+offset], al
    return mem.EmitCodeListing({0x00, 0x42, (uint8_t) offset});
  } else {
    // ADD byte[rdx+offset], al
    return mem.EmitCodeListing({0x00, 0x82}).and_then([&] {
      return mem.EmitCode((uint32_t) offset);
    });
  }
  // clang-format on
}

Err EmitDmullCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (0 == offset) {
    // This should never be created
    assert(0 != offset);
    return mem.EmitCodeListing({0x90});
  }
  Err e = Err::Ok();
  // clang-format off
  if (1 == amount) {
    // MOV al, byte[rdx]
    e = mem.EmitCodeListing({0x8A, 0x02});
  } else {
    // rdx needs to be saved, sinc MUL overwrites it
    e = Err::Ok().and_then([&] {
      // MOV r8, rdx
      return mem.EmitCodeListing({0x49, 0x89, 0xD0});
    }).and_then([&] {
      // MOV rax, amount
      return mem.EmitCodeListing({0x48, 0xC7, 0xC0}).and_then([&] { return mem.EmitCode((uint32_t) amount); });
    }).and_then([&] {
      // MOV bl, byte[rdx]
      return mem.EmitCodeListing({0x8A, 0x1A});
    }).and_then([&] {
      // MUL rbx
      return mem.EmitCodeListing({0x48, 0xF7, 0xE3});
    }).and_then([&] {
      // MOV rdx, r8
      return mem.EmitCodeListing({0x4C, 0x89, 0xC2});
    });
  }
  if (!e.IsOk()) {
    return e;
  }
  if (128 > offset && -128 < offset) {
    // SUB byte[rdx+offset], al
    return mem.EmitCodeListing({0x28, 0x42, (uint8_t) offset});
  } else {
    // SUB byte[rdx+offset], al
    return mem.EmitCodeListing({0x28, 0x82}).and_then([&] {
      return mem.EmitCode((uint32_t) offset);
    });
  }
  // clang-format on
}

Err EmitSetCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (0 == offset) {
    // MOV byte[rdx], amount
    return mem.EmitCodeListing({0xC6, 0x02, amount});
  } else if (128 > offset && -128 < offset) {
    // MOV byte[rdx+offset], amount
    return mem.EmitCodeListing({0xC6, 0x42, (uint8_t) offset, (uint8_t) amount});
  } else {
    // clang-format off
    // MOV byte[rdx+offset], amount
    return mem.EmitCodeListing({0xC6, 0x82})
      .and_then([&] {
        return mem.EmitCode((uint32_t) offset); })
      .and_then([&] {
        return mem.EmitCodeListing({amount});
      });
    // clang-format on
  }
}

Err EmitIncrPtr(CodeArea &mem, intptr_t amount) {
  // ADD rdx, amount
  if (128 > amount && -128 < amount) {
    return mem.EmitCodeListing({0x48, 0x83, 0xC2, (uint8_t) amount});
  } else {
    uint32_t a = (uint32_t) amount;
    return mem.EmitCodeListing({0x48, 0x81, 0xC2}).and_then([&] { return mem.EmitCode(a); });
  }
}

Err EmitDecrPtr(CodeArea &mem, intptr_t amount) {
  // SUB rdx, amount
  if (128 > amount && -128 < amount) {
    return mem.EmitCodeListing({0x48, 0x83, 0xEA, (uint8_t) amount});
  } else {
    uint32_t a = (uint32_t) amount;
    return mem.EmitCodeListing({0x48, 0x81, 0xEA}).and_then([&] { return mem.EmitCode(a); });
  }
}

Err EmitRead(CodeArea &mem, EOFMode eof_mode) {
  // clang-format off
  uintptr_t addr = (uintptr_t) do_read;
  return mem.EmitCodeListing({
      // PUSH rdx
      0x52,
      // MOV rax, addr
      0x48, 0xB8,
    }).and_then([&] { return mem.EmitCode64(addr);
    }).and_then([&] {
      return mem.EmitCodeListing({
#if defined(IS_WINDOWS)
          // MOV rcx, rdx
          0x48, 0x89, 0xD1,
          // MOV edi, eof_mode
          0xBF,
#endif
#if defined(IS_LINUX)
          // MOV rdi, rdx
          0x48, 0x89, 0xD7,
          // MOV esi, eof_mode
          0xBE,
#endif
        });
    }).and_then([&] { return mem.EmitCode((uint32_t) eof_mode);
    }).and_then([&] {
      return mem.EmitCodeListing({
          // CALL rax
          0xFF, 0xD0,
          // POP rdx
          0x5A
        });
    });
#if 0
  // Linux syscall version
  Err err = mem.EmitCodeListing({
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
      0x5A});
  // clang-format on
  if (!err.IsOk()) {
    return err;
  }
  switch (eof_mode) {
  case EOFMode::KEEP: {
    return err.Ok();
  } break;
  case EOFMode::ZERO: {
    // clang-format off
    return mem.EmitCodeListing({
        // CMP rax, 0x0
        0x48, 0x83, 0xF8, 0x00,
        // JNZ "next instruction"
        0x75, 0x03,
        // MOV byte[rdx], 0x0
        0xC6, 0x02, 0x00});
  } break;
  case EOFMode::NEG_ONE: {
    return mem.EmitCodeListing({
        // CMP rax, 0x0
        0x48, 0x83, 0xF8, 0x00,
        // JNZ "next instruction"
        0x75, 0x03,
        // MOV byte[rdx], -1
        0xC6, 0x02, 0xFF});
  } break;
  }
#endif
}

Err EmitWrite(CodeArea &mem) {
  // clang-format off
  uintptr_t addr = (uintptr_t) do_write;
  return mem.EmitCodeListing({
      // PUSH rdx
      0x52,
      // MOV rax, addr
      0x48, 0xB8,
    }).and_then([&] { return mem.EmitCode64(addr);
    }).and_then([&] {
      return mem.EmitCodeListing({
#if defined(IS_WINDOWS)
          // MOV rcx, rdx
          0x48, 0x89, 0xD1,
#endif
#if defined(IS_LINUX)
          // MOV rdi, rdx
          0x48, 0x89, 0xD7,
#endif
          // CALL rax
          0xFF, 0xD0,
          // POP rdx
          0x5A
        });
    });
#if 0
  // Linux syscall version
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
#endif
  // clang-format on
}

Err EmitJumpZero(CodeArea &mem) {
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

Err PatchJumpZero(CodeArea &mem, uint8_t *position, uintptr_t offset) {
  intptr_t signed_offset = (intptr_t) offset;
  if (signed_offset > (intptr_t) INT32_MAX || signed_offset < (intptr_t) INT32_MIN) {
    return Err::CodeInvalidOffset();
  }
  uint32_t offset32 = (uint32_t) offset;
  return mem.PatchCode(position - 4, offset32);
}

Err EmitJumpNonZero(CodeArea &mem) {
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

Err PatchJumpNonZero(CodeArea &mem, uint8_t *position, uintptr_t offset) {
  intptr_t signed_offset = (intptr_t) offset;
  if (signed_offset > (intptr_t) INT32_MAX || signed_offset < (intptr_t) INT32_MIN) {
    return Err::CodeInvalidOffset();
  }
  uint32_t offset32 = (uint32_t) offset;
  return mem.PatchCode(position - 4, offset32);
}

Err EmitFindCellHigh(CodeArea &mem, uint8_t value, uintptr_t move_size) {
  if (move_size >= (uintptr_t) UINT32_MAX) {
    return Err::CodeInvalidOffset();
  }
  // clang-format off
  return mem.EmitCodeListing({
      // CMP byte[rdx], value
      0x80, 0x3A, value,
      // JE "to the end"
      0x74, 0x09,
      // ADD rdx, move_size
      0x48, 0x81, 0xC2
    }).and_then([&] {
      return mem.EmitCode((uint32_t) move_size);
    }).and_then([&] {
      // JMP "back to SUB"
      return mem.EmitCodeListing({0xEB, 0xF2});
    });
  // clang-format on
}

Err EmitFindCellLow(CodeArea &mem, uint8_t value, uintptr_t move_size) {
  if (move_size >= (uintptr_t) UINT32_MAX) {
    return Err::CodeInvalidOffset();
  }
  // clang-format off
  return mem .EmitCodeListing({
      // CMP byte[rdx], value
      0x80, 0x3A, value,
      // JE "to the end"
      0x74, 0x09,
      // SUB rdx, move_size
      0x48, 0x81, 0xEA
    }).and_then([&] {
      return mem.EmitCode((uint32_t) move_size);
    }).and_then([&] {
      // JMP "back to SUB"
      return mem.EmitCodeListing({0xEB, 0xF2});
    });
  // clang-format on
}