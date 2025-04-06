// SPDX-License-Identifier: MIT License
#include "assembler.h"
#include "platform.h"

#if defined(IS_X86_64)
#include "debug.h"
#include "error.h"

/* ABI information

   ==== Linux System V ====

   Arguments: rdi, rsi, rdx, rcx, r8, r9
   Volatile registers: rax, rcx, rdx, rsi, rdi, r8, r9, r10, r11
   Frame pointer: rbp
   Stack pointer: rsp

   ==== Windows x64 ====

   Arguments: rcx, rdx, r8, r9
   Volatile registers: rax, rcx, rdx, r8, r9, r10, r11
   Frame pointer: rbp
   Stack pointer: rsp

   The ABI defines a shadow spaces on the stack, which is a region
   right above the return address on the stack which is owned by the
   called function and can be used to either store function parameters
   (for the debugger) or as a local scratch space.
   The caller is responsible for providing the storage and the callee
   can use it.
 */

void EmitEntry(CodeArea &mem) {
  // clang-format off
  // Just to be save, push ALL registers
  mem.EmitCodeListing({
      0x55,        // PUSH rbp
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
      0x54,        // PUSH rsp
      0x53,        // PUSH rbx
      0x52,        // PUSH rdx
      0x51,        // PUSH rcx
      0x50,        // PUSH rax
      // rdx is used to hold the pointer to the current cell
      // move the argument
#if defined(IS_WINDOWS)
      // MOV rdx, rcx
      0x48, 0x89, 0xCA,
#endif
#if defined(IS_LINUX)
      // MOV rdx, rdi
      0x48, 0x89, 0xFA,
#endif
      // set the frame pointer for later use
      // MOV rbp, rsp
      0x48, 0x89, 0xE5,
      // allocate some space on the stack for saving rdx across
      // calls and for windows shadow space
      // SUB rsp, 80
      0x48, 0x83, 0xEC, 0x50,
    });
  // clang-format on
}

void EmitExit(CodeArea &mem) {
  // clang-format off
  mem.EmitCodeListing({
      // Adjust the stack pointer back
      // ADD rsp, 80
      0x48, 0x83, 0xC4, 0x50,
      // pop all saved registers
      0x58,        // POP rax
      0x59,        // POP rcx
      0x5A,        // POP rdx
      0x5B,        // POP rbx
      0x5C,        // POP rsp
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
      0x5D,        // POP rbp
      0xC3         // ret
    });
  // clang-format on
}

void EmitNop(CodeArea &mem) {
  // Only used for debugging, so emit a bit more NOPs
  mem.EmitCodeListing({0x90, 0x90, 0x90, 0x90});
}

void EmitIncrCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (0 == offset) {
    // ADD byte[rdx], amount
    mem.EmitCodeListing({0x80, 0x02, amount});
  } else if (128 > offset && -128 < offset) {
    // ADD byte[rdx+offset], amount
    mem.EmitCodeListing({0x80, 0x42, (uint8_t) offset, (uint8_t) amount});
  } else {
    // ADD byte[rdx+offset], amount
    mem.EmitCodeListing({0x80, 0x82});
    mem.EmitCode((uint32_t) offset);
    mem.EmitCodeListing({amount});
  }
}

void EmitDecrCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (offset == 0) {
    // SUB byte[rdx], amount
    mem.EmitCodeListing({0x80, 0x2A, amount});
  } else if (128 > offset && -128 < offset) {
    // SUB byte[rdx+offset], amount
    mem.EmitCodeListing({0x80, 0x6A, (uint8_t) offset, (uint8_t) amount});
  } else {
    // SUB byte[rdx+offset], amount
    mem.EmitCodeListing({0x80, 0xAA});
    mem.EmitCode((uint32_t) offset);
    mem.EmitCodeListing({amount});
  }
}

void EmitImullCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  GUARANTEE(offset != 0, "Imull with offset %zd", offset);
  if (1 == amount) {
    // MOV al, byte[rdx]
    mem.EmitCodeListing({0x8A, 0x02});
  } else {
    // rdx needs to be saved, sinc MUL overwrites it
    // MOV r8, rdx
    mem.EmitCodeListing({0x49, 0x89, 0xD0});
    // MOV rax, amount
    mem.EmitCodeListing({0x48, 0xC7, 0xC0});
    mem.EmitCode((uint32_t) amount);
    // MOV bl, byte[rdx]
    mem.EmitCodeListing({0x8A, 0x1A});
    // MUL rbx
    mem.EmitCodeListing({0x48, 0xF7, 0xE3});
    // MOV rdx, r8
    mem.EmitCodeListing({0x4C, 0x89, 0xC2});
  }
  if (128 > offset && -128 < offset) {
    // ADD byte[rdx+offset], al
    mem.EmitCodeListing({0x00, 0x42, (uint8_t) offset});
  } else {
    // ADD byte[rdx+offset], al
    mem.EmitCodeListing({0x00, 0x82});
    mem.EmitCode((uint32_t) offset);
  }
}

void EmitDmullCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  GUARANTEE(offset != 0, "Imull with offset %zd", offset);
  if (1 == amount) {
    // MOV al, byte[rdx]
    mem.EmitCodeListing({0x8A, 0x02});
  } else {
    // rdx needs to be saved, sinc MUL overwrites it
    // MOV r8, rdx
    mem.EmitCodeListing({0x49, 0x89, 0xD0});
    // MOV rax, amount
    mem.EmitCodeListing({0x48, 0xC7, 0xC0});
    mem.EmitCode((uint32_t) amount);
    // MOV bl, byte[rdx]
    mem.EmitCodeListing({0x8A, 0x1A});
    // MUL rbx
    mem.EmitCodeListing({0x48, 0xF7, 0xE3});
    // MOV rdx, r8
    mem.EmitCodeListing({0x4C, 0x89, 0xC2});
  }
  if (128 > offset && -128 < offset) {
    // SUB byte[rdx+offset], al
    mem.EmitCodeListing({0x28, 0x42, (uint8_t) offset});
  } else {
    // SUB byte[rdx+offset], al
    mem.EmitCodeListing({0x28, 0x82});
    mem.EmitCode((uint32_t) offset);
  }
}

void EmitSetCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (0 == offset) {
    // MOV byte[rdx], amount
    mem.EmitCodeListing({0xC6, 0x02, amount});
  } else if (128 > offset && -128 < offset) {
    // MOV byte[rdx+offset], amount
    mem.EmitCodeListing({0xC6, 0x42, (uint8_t) offset, (uint8_t) amount});
  } else {
    // MOV byte[rdx+offset], amount
    mem.EmitCodeListing({0xC6, 0x82});
    mem.EmitCode((uint32_t) offset);
    mem.EmitCodeListing({amount});
  }
}

void EmitIncrPtr(CodeArea &mem, intptr_t amount) {
  // ADD rdx, amount
  if (128 > amount && -128 < amount) {
    mem.EmitCodeListing({0x48, 0x83, 0xC2, (uint8_t) amount});
  } else {
    uint32_t a = (uint32_t) amount;
    mem.EmitCodeListing({0x48, 0x81, 0xC2});
    mem.EmitCode(a);
  }
}

void EmitDecrPtr(CodeArea &mem, intptr_t amount) {
  // SUB rdx, amount
  if (128 > amount && -128 < amount) {
    mem.EmitCodeListing({0x48, 0x83, 0xEA, (uint8_t) amount});
  } else {
    uint32_t a = (uint32_t) amount;
    mem.EmitCodeListing({0x48, 0x81, 0xEA});
    mem.EmitCode(a);
  }
}

void EmitRead(CodeArea &mem, EOFMode eof_mode) {
  uintptr_t addr = (uintptr_t) bf_read;
  mem.EmitCodeListing({
      // Save rdx
      // MOV [rbp], rdx
      0x48,
      0x89,
      0x55,
      0x00,
      // MOV rax, addr
      0x48,
      0xB8,
  });
  mem.EmitCode64(addr);
  mem.EmitCodeListing({
#if defined(IS_WINDOWS)
      // MOV rcx, rdx
      0x48,
      0x89,
      0xD1,
      // MOV edx, eof_mode
      0xBA,
#endif
#if defined(IS_LINUX)
      // MOV rdi, rdx
      0x48,
      0x89,
      0xD7,
      // MOV esi, eof_mode
      0xBE,
#endif
  });
  mem.EmitCode((uint32_t) eof_mode);
  mem.EmitCodeListing({
      // CALL rax
      0xFF,
      0xD0,
      // restore rdx
      // MOV rdx, [rbp]
      0x48,
      0x8B,
      0x55,
      0x00,
  });
}

void EmitWrite(CodeArea &mem) {
  uintptr_t addr = (uintptr_t) bf_write;
  mem.EmitCodeListing({
      // Save rdx
      // MOV [rbp], rdx
      0x48,
      0x89,
      0x55,
      0x00,
      // MOV rax, addr
      0x48,
      0xB8,
  });
  mem.EmitCode64(addr);
  mem.EmitCodeListing({
#if defined(IS_WINDOWS)
      // MOV rcx, rdx
      0x48,
      0x89,
      0xD1,
#endif
#if defined(IS_LINUX)
      // MOV rdi, rdx
      0x48,
      0x89,
      0xD7,
#endif
      // CALL rax
      0xFF,
      0xD0,
      // restore rdx
      // MOV rdx, [rbp]
      0x48,
      0x8B,
      0x55,
      0x00,
  });
}

void EmitJumpZero(CodeArea &mem) {
  mem.EmitCodeListing({// CMP byte[rdx], 0
                       0x80,
                       0x3A,
                       0x00,
                       // JZ
                       0x0F,
                       0x84,
                       // Jump will be patched later
                       0x00,
                       0x00,
                       0x00,
                       0x00});
}

void PatchJumpZero(CodeArea &mem, uint8_t *position, uintptr_t offset) {
  intptr_t signed_offset = (intptr_t) offset;
  GUARANTEE(
      signed_offset <= (intptr_t) INT32_MAX && signed_offset >= (intptr_t) INT32_MIN, "Jump too large: %zd", offset);
  uint32_t offset32 = (uint32_t) offset;
  mem.PatchCode(position - 4, offset32);
}

void EmitJumpNonZero(CodeArea &mem) {
  mem.EmitCodeListing({// CMP byte[rdx], 0
                       0x80,
                       0x3A,
                       0x00,
                       // JNZ
                       0x0F,
                       0x85,
                       // Jump will be patched later
                       0x00,
                       0x00,
                       0x00,
                       0x00});
}

void PatchJumpNonZero(CodeArea &mem, uint8_t *position, uintptr_t offset) {
  intptr_t signed_offset = (intptr_t) offset;
  GUARANTEE(
      signed_offset <= (intptr_t) INT32_MAX && signed_offset >= (intptr_t) INT32_MIN, "Jump too large: %zd", offset);
  uint32_t offset32 = (uint32_t) offset;
  mem.PatchCode(position - 4, offset32);
}

void EmitFindCellHigh(CodeArea &mem, uint8_t value, uintptr_t move_size) {
  GUARANTEE(move_size < (uintptr_t) UINT32_MAX, "move_size too large: %zu", move_size);
  mem.EmitCodeListing({
      // CMP byte[rdx], value
      0x80,
      0x3A,
      value,
      // JE "to the end"
      0x74,
      0x09,
      // ADD rdx, move_size
      0x48,
      0x81,
      0xC2,
  });
  mem.EmitCode((uint32_t) move_size);
  // JMP "back to SUB"
  mem.EmitCodeListing({0xEB, 0xF2});
}

void EmitFindCellLow(CodeArea &mem, uint8_t value, uintptr_t move_size) {
  GUARANTEE(move_size < (uintptr_t) UINT32_MAX, "move_size too large: %zu", move_size);
  mem.EmitCodeListing({
      // CMP byte[rdx], value
      0x80,
      0x3A,
      value,
      // JE "to the end"
      0x74,
      0x09,
      // SUB rdx, move_size
      0x48,
      0x81,
      0xEA,
  });
  mem.EmitCode((uint32_t) move_size);
  // JMP "back to SUB"
  mem.EmitCodeListing({0xEB, 0xF2});
}

#endif
