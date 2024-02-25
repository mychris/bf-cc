// SPDX-License-Identifier: MIT License
#include "compiler.h"

#include <cassert>
#include <vector>

#include "assembler.h"
#include "error.h"

Err Compiler::Compile(OperationStream &stream, EOFMode eof_mode) noexcept {
  std::vector<std::pair<const Operation *, uint8_t *>> jump_list{};
  std::vector<std::pair<const Operation *, uint8_t *>> label_list{};
  Err err = Err::Ok();
  void *entry = m.mem->CurrentWriteAddr();
  m.entry = nullptr;
  err = EmitEntry(*m.mem);
  if (!err.IsOk()) {
    return err;
  }
  for (const Operation *op : stream) {
    switch (op->OpCode()) {
    case Instruction::NOP:
      err = EmitNop(*m.mem);
      break;
    case Instruction::INCR_CELL:
      err = EmitIncrCell(*m.mem, (uint8_t) op->Operand1(), op->Operand2());
      break;
    case Instruction::DECR_CELL:
      err = EmitDecrCell(*m.mem, (uint8_t) op->Operand1(), op->Operand2());
      break;
    case Instruction::IMUL_CELL:
      err = EmitImullCell(*m.mem, (uint8_t) op->Operand1(), op->Operand2());
      break;
    case Instruction::DMUL_CELL:
      err = EmitDmullCell(*m.mem, (uint8_t) op->Operand1(), op->Operand2());
      break;
    case Instruction::SET_CELL:
      err = EmitSetCell(*m.mem, (uint8_t) op->Operand1(), op->Operand2());
      break;
    case Instruction::INCR_PTR:
      err = EmitIncrPtr(*m.mem, op->Operand1());
      break;
    case Instruction::DECR_PTR:
      err = EmitDecrPtr(*m.mem, op->Operand1());
      break;
    case Instruction::READ:
      // clang-format off
      err = EmitIncrPtr(*m.mem, op->Operand2())
        .and_then([&]() {
          return EmitRead(*m.mem, eof_mode);
        })
        .and_then([&]() {
          return EmitDecrPtr(*m.mem, op->Operand2());
        });
      // clang-format on
      break;
    case Instruction::WRITE:
      // clang-format off
      err = EmitIncrPtr(*m.mem, op->Operand2())
        .and_then([&]() {
          return EmitWrite(*m.mem);
        })
        .and_then([&]() {
          return EmitDecrPtr(*m.mem, op->Operand2());
        });
      // clang-format on
      break;
    case Instruction::JZ:
      err = EmitJumpZero(*m.mem);
      jump_list.push_back({op, m.mem->CurrentWriteAddr()});
      break;
    case Instruction::JNZ:
      err = EmitJumpNonZero(*m.mem);
      jump_list.push_back({op, m.mem->CurrentWriteAddr()});
      break;
    case Instruction::LABEL:
      label_list.push_back({op, m.mem->CurrentWriteAddr()});
      break;
    case Instruction::FIND_CELL_HIGH:
      err = EmitFindCellHigh(*m.mem, (uint8_t) op->Operand1(), (uintptr_t) op->Operand2());
      break;
    case Instruction::FIND_CELL_LOW:
      err = EmitFindCellLow(*m.mem, (uint8_t) op->Operand1(), (uintptr_t) op->Operand2());
      break;
    }
    if (!err.IsOk()) {
      return err;
    }
  }
  // Patch the jumps
  for (const auto &[jump, code_pos] : jump_list) {
    uint8_t *target_pos{nullptr};
    for (const auto &[label, label_pos] : label_list) {
      if (jump->Operand1() == (Operation::operand_type) label) {
        target_pos = label_pos;
        break;
      }
    }
    assert(target_pos > (uint8_t *) 0 && "Label not found");
    assert(jump->IsAny({Instruction::JZ, Instruction::JNZ}) && "Invalid op code in jump list");
    if (jump->Is(Instruction::JZ)) {
      err = PatchJumpZero(*m.mem, code_pos, (uintptr_t) (target_pos - code_pos));
    } else if (jump->Is(Instruction::JNZ)) {
      err = PatchJumpNonZero(*m.mem, code_pos, (uintptr_t) (target_pos - code_pos));
    }
    if (!err.IsOk()) {
      return err;
    }
  }
  err = EmitExit(*m.mem);
  if (!err.IsOk()) {
    return err;
  }
  err = m.mem->MakeExecutable();
  if (!err.IsOk()) {
    return err;
  }
  m.entry = reinterpret_cast<CodeEntry>(entry);
  return Err::Ok();
}
