// SPDX-License-Identifier: MIT License
#include "assembler.h"
#include "platform.h"

#if defined(IS_AARCH64)
#include <cstdint>

#include "debug.h"
#include "error.h"

#define __ Encoder::

enum class R : uint32_t {
  // clang-format off
  X0 = 0,
  X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11,
  X12, X13, X14, X15, X16, X17, X18, X19, X20, X21,
  X22, X23, X24, X25, X26, X27, X28, X29, X30, X31,
  W0, W1, W2, W3, W4, W5, W6, W7, W8, W9, W10, W11,
  W12, W13, W14, W15, W16, W17, W18, W19, W20, W21,
  W22, W23, W24, W25, W26, W27, W28, W29, W30, W31,
  FP, LR, SP,
  // clang-format on
};

static const R R_CELL = R::X19;
static const R R_READ = R::X20;
static const R R_WRITE = R::X21;
static const R R_TMPX1 = R::X1;
static const R R_TMPX2 = R::X2;
static const R R_TMPX3 = R::X3;
static const R R_TMPW1 = R::W1;
static const R R_TMPW2 = R::W2;
static const R R_TMPW3 = R::W3;

class Encoder {
private:
  static inline uint32_t NormReg(R reg, uint32_t *sf_out) noexcept {
    switch (reg) {
    case R::FP:
      reg = R::X29;
      break;
    case R::LR:
      reg = R::X30;
      break;
    case R::SP:
      reg = R::X31;
      break;
    default:
      break;
    }
    uint32_t r = static_cast<uint32_t>(reg);
    uint32_t sf = 1;
    if (r >= static_cast<uint32_t>(R::W0)) {
      sf = 0;
      r -= static_cast<uint32_t>(R::W0);
    }
    if (sf_out) *sf_out = sf;
    return r & 0b11111;
  }

public:
  static constexpr uint32_t NOP() noexcept {
    return 0x1F2003D5;
  }

  static constexpr uint32_t BRK(uint16_t imm = 0x1000) noexcept {
    uint32_t op = 0b11010100001000000000000000000000;
    op |= static_cast<uint32_t>(imm) << 5;
    return op;
  }

  static constexpr uint32_t MOV(R regd, R regn) noexcept {
    uint32_t op = 0b00101010000000000000001111100000;
    uint32_t sf = 1;
    uint32_t rd = NormReg(regd, &sf);
    uint32_t rn = NormReg(regn, nullptr);
    op |= sf << 31;
    op |= rn << 16;
    op |= rd;
    return op;
  }

  static constexpr uint32_t MOVZ(R regd, uint16_t imm, uint8_t hw = 0) noexcept {
    uint32_t op = 0b01010010100000000000000000000000;
    uint32_t sf = 1;
    uint32_t rd = NormReg(regd, &sf);
    ASSERT(hw < 0b100, "Invalid MOVZ shift amount: %u", hw);
    ASSERT(sf == 1 || hw < 0b10, "Invalid MOVZ shift amount: %u", hw);
    op |= sf << 31;
    op |= static_cast<uint32_t>(hw & 0b11) << 21;
    op |= static_cast<uint32_t>(imm) << 5;
    op |= rd;
    return op;
  }

  static constexpr uint32_t MOVK(R regd, uint16_t imm, uint8_t hw = 0) noexcept {
    uint32_t op = 0b01110010100000000000000000000000;
    uint32_t sf = 1;
    uint32_t rd = NormReg(regd, &sf);
    ASSERT(hw < 0b100, "Invalid MOVK shift amount: %u", hw);
    ASSERT(sf == 1 || hw < 0b10, "Invalid MOVK shift amount: %u", hw);
    op |= sf << 31;
    op |= static_cast<uint32_t>(hw & 0b11) << 21;
    op |= static_cast<uint32_t>(imm) << 5;
    op |= rd;
    return op;
  }

  static constexpr uint32_t ADD(R regd, R regn, R regm) noexcept {
    uint32_t op = 0b00001011000000000000000000000000;
    uint32_t sf = 1;
    uint32_t rd = NormReg(regd, &sf);
    uint32_t rn = NormReg(regn, nullptr);
    uint32_t rm = NormReg(regm, nullptr);
    op |= sf << 31;
    op |= rm << 16;
    op |= rn << 5;
    op |= rd;
    return op;
  }

  static constexpr uint32_t ADD(R regd, R regn, uint16_t imm, bool shift_12 = false) noexcept {
    ASSERT(imm < 0x1000, "Invalid ADD immediate: %u", imm);
    uint32_t op = 0b00010001000000000000000000000000;
    uint32_t sf = 1;
    uint32_t rd = NormReg(regd, &sf);
    uint32_t rn = NormReg(regn, nullptr);
    op |= sf << 31;
    if (shift_12) {
      op |= 1 << 22;
    }
    op |= static_cast<uint32_t>(imm & 0xFFF) << 10;
    op |= rn << 5;
    op |= rd;
    return op;
  }

  static constexpr uint32_t SUB(R regd, R regn, R regm) noexcept {
    uint32_t op = 0b01001011000000000000000000000000;
    uint32_t sf = 1;
    uint32_t rd = NormReg(regd, &sf);
    uint32_t rn = NormReg(regn, nullptr);
    uint32_t rm = NormReg(regm, nullptr);
    op |= sf << 31;
    op |= rm << 16;
    op |= rn << 5;
    op |= rd;
    return op;
  }

  static constexpr uint32_t SUB(R regd, R regn, uint16_t imm, bool shift_12 = false) noexcept {
    ASSERT(imm < 0x1000, "Invalid SUB immediate: %u", imm);
    uint32_t op = 0b01010001000000000000000000000000;
    uint32_t sf = 1;
    uint32_t rd = NormReg(regd, &sf);
    uint32_t rn = NormReg(regn, nullptr);
    op |= sf << 31;
    if (shift_12) {
      op |= 1 << 22;
    }
    op |= static_cast<uint32_t>(imm & 0xFFF) << 10;
    op |= rn << 5;
    op |= rd;
    return op;
  }

  static constexpr uint32_t MUL(R regd, R regn, R regm) noexcept {
    uint32_t op = 0b00011011000000000111110000000000;
    uint32_t sf = 1;
    uint32_t rd = NormReg(regd, &sf);
    uint32_t rn = NormReg(regn, nullptr);
    uint32_t rm = NormReg(regm, nullptr);
    op |= sf << 31;
    op |= rm << 16;
    op |= rn << 5;
    op |= rd;
    return op;
  }

  static constexpr uint32_t MADD(R regd, R regn, R regm, R rega) noexcept {
    uint32_t op = 0b00011011000000000000000000000000;
    uint32_t sf = 1;
    uint32_t rd = NormReg(regd, &sf);
    uint32_t rn = NormReg(regn, nullptr);
    uint32_t rm = NormReg(regm, nullptr);
    uint32_t ra = NormReg(rega, nullptr);
    op |= sf << 31;
    op |= rm << 16;
    op |= ra << 10;
    op |= rn << 5;
    op |= rd;
    return op;
  }

  static constexpr uint32_t MSUB(R regd, R regn, R regm, R rega) noexcept {
    uint32_t op = 0b00011011000000001000000000000000;
    uint32_t sf = 1;
    uint32_t rd = NormReg(regd, &sf);
    uint32_t rn = NormReg(regn, nullptr);
    uint32_t rm = NormReg(regm, nullptr);
    uint32_t ra = NormReg(rega, nullptr);
    op |= sf << 31;
    op |= rm << 16;
    op |= ra << 10;
    op |= rn << 5;
    op |= rd;
    return op;
  }

  static constexpr uint32_t EOR(R regd, R regn, R regm) noexcept {
    uint32_t op = 0b01001010000000000000000000000000;
    uint32_t sf = 1;
    uint32_t rd = NormReg(regd, &sf);
    uint32_t rn = NormReg(regn, nullptr);
    uint32_t rm = NormReg(regm, nullptr);
    op |= sf << 31;
    op |= rm << 16;
    op |= rn << 5;
    op |= rd;
    return op;
  }

  static constexpr uint32_t CMP(R regn, uint16_t imm, bool shift_12 = false) noexcept {
    uint32_t op = 0b01110001000000000000000000011111;
    uint32_t sf = 1;
    uint32_t rn = NormReg(regn, nullptr);
    ASSERT(imm <= 0xFFF, "Invalid CMP immediate: %u", imm);
    op |= sf << 31;
    if (shift_12) {
      op |= 1 << 22;
    }
    op |= static_cast<uint32_t>(imm & 0xFFF) << 10;
    op |= rn << 5;
    return op;
  }

  static constexpr uint32_t LDRB(R regt, R regn, uint16_t imm = 0) noexcept {
    uint32_t op = 0b00111001010000000000000000000000;
    uint32_t rt = NormReg(regt, nullptr);
    uint32_t rn = NormReg(regn, nullptr);
    ASSERT(imm <= 0xFFF, "Invalid LDRB immediate: %u", imm);
    op |= static_cast<uint32_t>(imm & 0xFFF) << 10u;
    op |= rn << 5;
    op |= rt;
    return op;
  }

  static constexpr uint32_t STRB(R regt, R regn, uint16_t imm = 0) noexcept {
    uint32_t op = 0b00111001000000000000000000000000;
    uint32_t rt = NormReg(regt, nullptr);
    uint32_t rn = NormReg(regn, nullptr);
    ASSERT(imm <= 0xFFF, "Invalid STRB immediate: %u", imm);
    op |= static_cast<uint32_t>(imm & 0xFFF) << 10;
    op |= rn << 5;
    op |= rt;
    return op;
  }

  static constexpr uint32_t B(int32_t imm) noexcept {
    uint32_t op = 0b00010100000000000000000000000000;
    ASSERT(imm <= INT32_C(0x2000000) && imm >= INT32_C(-0x2000000), "Invalid B immediate: %d", imm);
    op |= static_cast<uint32_t>(imm) & 0x3ffffff;
    return op;
  }

  static constexpr uint32_t BEQ(int32_t imm) noexcept {
    uint32_t op = 0b01010100000000000000000000000000;
    ASSERT(imm <= 0x7FFFF || imm >= -0x80000, "Invalid BEQ immediate: %d", imm);
    op |= (static_cast<uint32_t>(imm) & 0x7FFFF) << 5;
    return op;
  }

  static constexpr uint32_t BNE(int32_t imm) noexcept {
    uint32_t op = 0b01010100000000000000000000000001;
    ASSERT(imm <= 0x7FFFF || imm >= -0x80000, "Invalid BNE immediate: %d", imm);
    op |= (static_cast<uint32_t>(imm) & 0x7FFFF) << 5;
    return op;
  }

  static constexpr uint32_t BLR(R regn) noexcept {
    uint32_t op = 0b11010110001111110000000000000000;
    uint32_t rn = NormReg(regn, nullptr);
    return op | (rn << 5);
  }
};

/* ABI information

   ==== Linux System V ====

   ==== Internal ====
   r19: cell pointer
   r20: address of bf_write
   r21: address of bf_read
   r0: tmp1 register
   r1: tmp2 register
   r2: tmp3 register
 */

void LoadImmediate32(CodeArea &mem, R target, uint32_t value) {
  mem.EmitCode(__ MOVZ(target, static_cast<uint16_t>(value & 0xFFFF)));
  if (value > 0xFFFF) {
    mem.EmitCode(__ MOVK(target, static_cast<uint16_t>((value >> 16) & 0xFFFF), 1));
  }
}

void LoadImmediate64(CodeArea &mem, R target, uint64_t value) {
  mem.EmitCode(__ MOVZ(target, static_cast<uint16_t>(value & 0xFFFF)));
  if (value > UINT64_C(0xFFFF)) {
    uint16_t v = static_cast<uint16_t>((value >> 16) & 0xFFFF);
    if (v > 0) {
      mem.EmitCode(__ MOVK(target, v, 1));
    }
  }
  if (value > UINT64_C(0xFFFFFFFF)) {
    uint16_t v = static_cast<uint16_t>((value >> 32) & 0xFFFF);
    if (v > 0) {
      mem.EmitCode(__ MOVK(target, v, 2));
    }
  }
  if (value > UINT64_C(0xFFFFFFFFFFFF)) {
    mem.EmitCode(__ MOVK(target, static_cast<uint16_t>((value >> 48) & 0xFFFF), 3));
  }
}

void EmitEntry(CodeArea &mem) {
  // clang-format off
  mem.EmitCodeListing({
      // Save registers
      // STP x29, x30, [sp, -16]!
      0xFD, 0x7B, 0xBF, 0xA9,
      // STP x19, x20, [sp, -16]!
      0xF3, 0x53, 0xBF, 0xA9,
      // STP x21, x22, [sp, -16]!
      0xF5, 0x5B, 0xBF, 0xA9,
      // Set FP
      // MOV x29, sp
      0xFD, 0x03, 0x00, 0x91,
      // MOV x19, x0
      0xF3, 0x03, 0x00, 0xAA,
    });
  LoadImmediate64(mem, R_WRITE, (uintptr_t) bf_write);
  LoadImmediate64(mem, R_READ, (uintptr_t) bf_read);
  // clang-format on
}

void EmitExit(CodeArea &mem) {
  // clang-format off
  mem.EmitCodeListing({
      // LDP x21, x22, [sp], 16
      0xF5, 0x5B, 0xC1, 0xA8,
      // LDP x19, x20, [sp], 16
      0xF3, 0x53, 0xC1, 0xA8,
      // LDP x29, x30, [sp], 16
      0xFD, 0x7B, 0xC1, 0xA8,
      // RET
      0xC0, 0x03, 0x5F, 0xD6,
    });
  // clang-format on
}

void EmitNop(CodeArea &mem) {
  // Only used for debugging, so emit a bit more NOPs
  mem.EmitCode(__ NOP());
  mem.EmitCode(__ NOP());
  mem.EmitCode(__ NOP());
  mem.EmitCode(__ NOP());
}

static void EmitIncrDecrCell(CodeArea &mem, uint8_t amount, intptr_t offset, bool is_incr) {
  ASSERT(offset < INT32_MAX && offset > INT32_MIN, "check");
  R cell_reg = R_CELL;
  uint16_t cell_reg_offset = static_cast<uint16_t>(offset);
  if (offset < 0 || offset > 0xFFF) {
    cell_reg = R_TMPX1;
    cell_reg_offset = 0;
    LoadImmediate64(mem, cell_reg, static_cast<uint64_t>(offset));
    mem.EmitCode(__ ADD(cell_reg, R_CELL, cell_reg));
  }
  mem.EmitCode(__ LDRB(R_TMPW2, cell_reg, cell_reg_offset));
  if (is_incr) {
    mem.EmitCode(__ ADD(R_TMPW2, R_TMPW2, amount));
  } else {
    mem.EmitCode(__ SUB(R_TMPW2, R_TMPW2, amount));
  }
  mem.EmitCode(__ STRB(R_TMPW2, cell_reg, cell_reg_offset));
}

void EmitIncrCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  EmitIncrDecrCell(mem, amount, offset, true);
}

void EmitDecrCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  EmitIncrDecrCell(mem, amount, offset, false);
}

void EmitImullCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  ASSERT(0 != offset, "check");
  R mul_amount = R::W1;
  R cur = R::W2;
  R orig = R::W4;
  R reg_offset = R::X3;
  R reg_source_cell = R_CELL;
  R reg_target_cell = R::X6;
  mem.EmitCode(__ LDRB(cur, reg_source_cell));
  LoadImmediate64(mem, reg_offset, static_cast<uint64_t>(offset));
  mem.EmitCode(__ ADD(reg_target_cell, reg_source_cell, reg_offset));
  mem.EmitCode(__ LDRB(orig, reg_target_cell));
  mem.EmitCode(__ MOVZ(mul_amount, amount));
  mem.EmitCode(__ MADD(cur, cur, mul_amount, orig));
  mem.EmitCode(__ STRB(cur, reg_target_cell));
}

void EmitDmullCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  ASSERT(0 != offset, "check");
  R mul_amount = R::W0;
  R cur = R::W1;
  R orig = R::W2;
  R reg_offset = R::X4;
  R reg_source_cell = R_CELL;
  R reg_target_cell = R::X6;
  mem.EmitCode(__ LDRB(cur, reg_source_cell));
  LoadImmediate64(mem, reg_offset, static_cast<uint64_t>(offset));
  mem.EmitCode(__ ADD(reg_target_cell, reg_source_cell, reg_offset));
  mem.EmitCode(__ LDRB(orig, reg_target_cell));
  mem.EmitCode(__ MOVZ(mul_amount, amount));
  mem.EmitCode(__ MSUB(cur, cur, mul_amount, orig));
  mem.EmitCode(__ STRB(cur, reg_target_cell));
}

void EmitSetCell(CodeArea &mem, uint8_t amount, intptr_t offset) {
  LoadImmediate32(mem, R_TMPW2, amount);
  if (offset < 0 || offset > 0xFFF) {
    LoadImmediate64(mem, R_TMPX1, static_cast<uint64_t>(offset));
    mem.EmitCode(__ ADD(R_TMPX1, R_CELL, R_TMPX1));
    mem.EmitCode(__ STRB(R_TMPW2, R_TMPX1));
  } else {
    mem.EmitCode(__ STRB(R_TMPW2, R_CELL, static_cast<uint16_t>(offset)));
  }
}

void EmitIncrPtr(CodeArea &mem, intptr_t amount) {
  if (amount < 0) {
    EmitDecrPtr(mem, -amount);
  } else {
    if (amount > 0xFFF) {
      LoadImmediate64(mem, R_TMPX1, static_cast<uint64_t>(amount));
      mem.EmitCode(__ ADD(R_CELL, R_CELL, R_TMPX1));
    } else if (amount != 0) {
      mem.EmitCode(__ ADD(R_CELL, R_CELL, static_cast<uint16_t>(amount)));
    }
  }
}

void EmitDecrPtr(CodeArea &mem, intptr_t amount) {
  if (amount < 0) {
    EmitIncrPtr(mem, -amount);
  } else {
    if (amount > 0xFFF) {
      LoadImmediate64(mem, R_TMPX1, static_cast<uint64_t>(amount));
      mem.EmitCode(__ SUB(R_CELL, R_CELL, R_TMPX1));
    } else if (amount != 0) {
      mem.EmitCode(__ SUB(R_CELL, R_CELL, static_cast<uint16_t>(amount)));
    }
  }
}

void EmitRead(CodeArea &mem, EOFMode eof_mode) {
  mem.EmitCode(__ MOV(R::X0, R_CELL));
  mem.EmitCode(__ MOVZ(R::W1, static_cast<uint16_t>(eof_mode)));
  mem.EmitCode(__ BLR(R_READ));
}

void EmitWrite(CodeArea &mem) {
  mem.EmitCode(__ MOV(R::X0, R_CELL));
  mem.EmitCode(__ BLR(R_WRITE));
}

static void PrepareJump(CodeArea &mem) {
  mem.EmitCode(__ LDRB(R_TMPW1, R_CELL));
  mem.EmitCode(__ CMP(R_TMPW1, 0));
  // Jump will be patched later
  mem.EmitCode(__ BRK());
}

static void PatchJump(CodeArea &mem, uint8_t *position, uintptr_t offset, bool is_beq) {
  ASSERT((offset & 0b11) == 0, "check");
  intptr_t signed_offset = (intptr_t) offset;
  signed_offset /= 4;
  signed_offset += 1;
  ASSERT(signed_offset <= 0x3ffff && signed_offset >= -0x3ffff, "check");
  if (is_beq) {
    mem.PatchCode(position - 4, __ BEQ((signed_offset)));
  } else {
    mem.PatchCode(position - 4, __ BNE((signed_offset)));
  }
}

void EmitJumpZero(CodeArea &mem) {
  PrepareJump(mem);
}

void PatchJumpZero(CodeArea &mem, uint8_t *position, uintptr_t offset) {
  PatchJump(mem, position, offset, true);
}

void EmitJumpNonZero(CodeArea &mem) {
  PrepareJump(mem);
}

void PatchJumpNonZero(CodeArea &mem, uint8_t *position, uintptr_t offset) {
  PatchJump(mem, position, offset, false);
}

void EmitFindCellHigh(CodeArea &mem, uint8_t value, uintptr_t move_size) {
  mem.EmitCode(__ B(2));
  mem.EmitCode(__ ADD(R_CELL, R_CELL, move_size));
  mem.EmitCode(__ LDRB(R_TMPW1, R_CELL));
  mem.EmitCode(__ CMP(R_TMPW1, value));
  mem.EmitCode(__ BNE(-3));
}

void EmitFindCellLow(CodeArea &mem, uint8_t value, uintptr_t move_size) {
  mem.EmitCode(__ B(2));
  mem.EmitCode(__ SUB(R_CELL, R_CELL, move_size));
  mem.EmitCode(__ LDRB(R_TMPW1, R_CELL));
  mem.EmitCode(__ CMP(R_TMPW1, value));
  mem.EmitCode(__ BNE(-3));
}

#endif
