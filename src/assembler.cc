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

static Err EmitIncrCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (0 == offset) {
    // ADD byte[rdx], amount
    return mem.EmitCodeListing({0x80, 0x02, amount});
  } else if (128 > offset && -128 < offset) {
    // ADD byte[rdx+offset], amount
    return mem.EmitCodeListing({0x80, 0x42, (uint8_t) offset, (uint8_t) amount});
  } else {
    // ADD byte[rdx+offset], amount
    return mem.EmitCodeListing({0x80, 0x82}).and_then([&] { return mem.EmitCode((uint32_t) offset); }).and_then([&] {
      return mem.EmitCodeListing({amount});
    });
  }
}

static Err EmitDecrCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (offset == 0) {
    // SUB byte[rdx], amount
    return mem.EmitCodeListing({0x80, 0x2A, amount});
  } else if (128 > offset && -128 < offset) {
    // SUB byte[rdx+offset], amount
    return mem.EmitCodeListing({0x80, 0x6A, (uint8_t) offset, (uint8_t) amount});
  } else {
    // SUB byte[rdx+offset], amount
    return mem.EmitCodeListing({0x80, 0xAA}).and_then([&] { return mem.EmitCode((uint32_t) offset); }).and_then([&] {
      return mem.EmitCodeListing({amount});
    });
  }
}

static Err EmitImullCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (0 == offset) {
    // This should never be created
    assert(0 != offset);
    return mem.EmitCodeListing({0x90});
  }
  // rdx needs to be saved, sinc MUL overwrites it
  // clang-format off
  Err e = Err::Ok().and_then([&] {
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
  if (!e.IsOk()) {
    return e;
  }
  if (128 > offset && -128 < offset) {
    // ADD byte[rdx], al
    return mem.EmitCodeListing({0x00, 0x42, (uint8_t) offset});
  } else {
    // ADD byte[rdx], al
    return mem.EmitCodeListing({0x4C, 0x89, 0xC2, 0x00, 0x82}).and_then([&] {
      return mem.EmitCode((uint32_t) offset);
    });
  }
  // clang-format on
}

static Err EmitSetCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  if (0 == offset) {
    // MOV byte[rdx], amount
    return mem.EmitCodeListing({0xC6, 0x02, amount});
  } else if (128 > offset && -128 < offset) {
    // MOV byte[rdx+offset], amount
    return mem.EmitCodeListing({0xC6, 0x42, (uint8_t) offset, (uint8_t) amount});
  } else {
    // MOV byte[rdx+offset], amount
    return mem.EmitCodeListing({0xC6, 0x82}).and_then([&] { return mem.EmitCode((uint32_t) offset); }).and_then([&] {
      return mem.EmitCodeListing({amount});
    });
  }
}

static Err EmitIncrPtr(CodeArea &mem, intptr_t amount) {
  // ADD rdx, amount
  if (128 > amount && -128 < amount) {
    return mem.EmitCodeListing({0x48, 0x83, 0xC2, (uint8_t) amount});
  } else {
    return mem.EmitCodeListing({0x48, 0x81, 0xC2}).and_then([&] { return mem.EmitCode((uint32_t) amount); });
  }
}

static Err EmitDecrPtr(CodeArea &mem, intptr_t amount) {
  // SUB rdx, amount
  if (128 > amount && -128 < amount) {
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

std::variant<CodeEntry, Err> AssemblerX8664::Assemble(OperationStream &stream) {
  std::vector<std::pair<Operation *, uint8_t *>> jump_list = {};
  Err err = Err::Ok();
  void *entry = m.mem.CurrentWriteAddr();
  auto iter = stream.Begin();
  auto end = stream.End();
  err = EmitEntry(m.mem);
  if (!err.IsOk()) {
    return err;
  }
  while (iter != end) {
    switch (iter->OpCode()) {
    case Instruction::ANY:
      // should not be in the stream!
      break;
    case Instruction::NOP:
      err = EmitNop(m.mem);
      break;
    case Instruction::INCR_CELL:
      err = EmitIncrCell(m.mem, (uint8_t) iter->Operand1(), iter->Operand2());
      break;
    case Instruction::DECR_CELL:
      err = EmitDecrCell(m.mem, (uint8_t) iter->Operand1(), iter->Operand2());
      break;
    case Instruction::IMUL_CELL:
      err = EmitImullCell(m.mem, (uint8_t) iter->Operand1(), iter->Operand2());
      break;
    case Instruction::SET_CELL:
      err = EmitSetCell(m.mem, (uint8_t) iter->Operand1(), iter->Operand2());
      break;
    case Instruction::INCR_PTR:
      err = EmitIncrPtr(m.mem, iter->Operand1());
      break;
    case Instruction::DECR_PTR:
      err = EmitDecrPtr(m.mem, iter->Operand1());
      break;
    case Instruction::READ:
      err = EmitIncrPtr(m.mem, iter->Operand2()).and_then([&]() { return EmitRead(m.mem); }).and_then([&]() {
        return EmitDecrPtr(m.mem, iter->Operand2());
      });
      break;
    case Instruction::WRITE:
      err = EmitIncrPtr(m.mem, iter->Operand2()).and_then([&]() { return EmitWrite(m.mem); }).and_then([&]() {
        return EmitDecrPtr(m.mem, iter->Operand2());
      });
      break;
    case Instruction::JUMP_ZERO:
      err = EmitJumpZero(m.mem);
      jump_list.push_back({*iter, m.mem.CurrentWriteAddr()});
      break;
    case Instruction::JUMP_NON_ZERO:
      err = EmitJumpNonZero(m.mem);
      jump_list.push_back({*iter, m.mem.CurrentWriteAddr()});
      break;
    case Instruction::FIND_CELL_HIGH:
      err = EmitFindCellHigh(m.mem, (uint8_t) iter->Operand1(), (uintptr_t) iter->Operand2());
      break;
    case Instruction::FIND_CELL_LOW:
      err = EmitFindCellLow(m.mem, (uint8_t) iter->Operand1(), (uintptr_t) iter->Operand2());
      break;
    }
    if (!err.IsOk()) {
      return err;
    }
    ++iter;
  }
  // Patch the jumps
  for (const auto &[jump, code_pos] : jump_list) {
    uint8_t *target_pos = 0;
    for (const auto &[target_jump, pos2] : jump_list) {
      if (jump->Operand1() == (Operation::operand_type) target_jump) {
        target_pos = pos2;
        break;
      }
    }
    assert(target_pos > (uint8_t *) 0 && "Jump destination not found");
    if (jump->OpCode() == Instruction::JUMP_ZERO) {
      err = PatchJumpZero(m.mem, code_pos, (uintptr_t) (target_pos - code_pos));
      if (!err.IsOk()) {
        return err;
      }
    } else if (jump->OpCode() == Instruction::JUMP_NON_ZERO) {
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
