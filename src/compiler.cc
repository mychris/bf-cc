// SPDX-License-Identifier: MIT License
#include "compiler.h"

#include <cstdio>
#include <vector>

#include "assembler.h"
#include "debug.h"
#include "error.h"

#define DEBUG_COMP(x)

Err Compiler::Compile(OperationStream &stream, EOFMode eof_mode) noexcept {
  std::vector<std::pair<const Operation *, uint8_t *>> jump_list{};
  std::vector<std::pair<const Operation *, uint8_t *>> label_list{};
  void *entry = m.mem->CurrentWriteAddr();
  m.entry = nullptr;
  EmitEntry(*m.mem);
  if (m.mem->HasWriteError()) {
    return Err::OutOfMemory();
  }
  for (const Operation *op : stream) {
    switch (op->OpCode()) {
    case Instruction::NOP:
      DEBUG_COMP(printf("NOP\n"));
      EmitNop(*m.mem);
      break;
    case Instruction::INCR_CELL:
      DEBUG_COMP(printf("INCR_CELL %zu %zu\n", op->Operand1(), op->Operand2()));
      EmitIncrCell(*m.mem, (uint8_t) op->Operand1(), op->Operand2());
      break;
    case Instruction::DECR_CELL:
      DEBUG_COMP(printf("DECR_CELL %zu %zu\n", op->Operand1(), op->Operand2()));
      EmitDecrCell(*m.mem, (uint8_t) op->Operand1(), op->Operand2());
      break;
    case Instruction::IMUL_CELL:
      DEBUG_COMP(printf("IMUL_CELL %zu %zu\n", op->Operand1(), op->Operand2()));
      EmitImullCell(*m.mem, (uint8_t) op->Operand1(), op->Operand2());
      break;
    case Instruction::DMUL_CELL:
      DEBUG_COMP(printf("DMUL_CELL %zu %zu\n", op->Operand1(), op->Operand2()));
      EmitDmullCell(*m.mem, (uint8_t) op->Operand1(), op->Operand2());
      break;
    case Instruction::SET_CELL:
      DEBUG_COMP(printf("SET_CELL %zu %zu\n", op->Operand1(), op->Operand2()));
      EmitSetCell(*m.mem, (uint8_t) op->Operand1(), op->Operand2());
      break;
    case Instruction::INCR_PTR:
      DEBUG_COMP(printf("INCR_PTR %zu\n", op->Operand1()));
      EmitIncrPtr(*m.mem, op->Operand1());
      break;
    case Instruction::DECR_PTR:
      DEBUG_COMP(printf("DECR_CELL %zu\n", op->Operand1()));
      EmitDecrPtr(*m.mem, op->Operand1());
      break;
    case Instruction::READ:
      DEBUG_COMP(printf("READ %zu %zu\n", op->Operand1(), op->Operand2()));
      EmitIncrPtr(*m.mem, op->Operand2());
      EmitRead(*m.mem, eof_mode);
      EmitDecrPtr(*m.mem, op->Operand2());
      break;
    case Instruction::WRITE:
      DEBUG_COMP(printf("WRITE %zu %zu\n", op->Operand1(), op->Operand2()));
      EmitIncrPtr(*m.mem, op->Operand2());
      EmitWrite(*m.mem);
      EmitDecrPtr(*m.mem, op->Operand2());
      break;
    case Instruction::JZ:
      DEBUG_COMP(printf("JZ\n"));
      EmitJumpZero(*m.mem);
      jump_list.push_back({op, m.mem->CurrentWriteAddr()});
      break;
    case Instruction::JNZ:
      DEBUG_COMP(printf("JNZ\n"));
      EmitJumpNonZero(*m.mem);
      jump_list.push_back({op, m.mem->CurrentWriteAddr()});
      break;
    case Instruction::LABEL:
      label_list.push_back({op, m.mem->CurrentWriteAddr()});
      break;
    case Instruction::FIND_CELL_HIGH:
      DEBUG_COMP(printf("FIND_CELL_HIGH %zu %zu\n", op->Operand1(), op->Operand2()));
      EmitFindCellHigh(*m.mem, (uint8_t) op->Operand1(), (uintptr_t) op->Operand2());
      break;
    case Instruction::FIND_CELL_LOW:
      DEBUG_COMP(printf("FIND_CELL_LOW %zu %zu\n", op->Operand1(), op->Operand2()));
      EmitFindCellLow(*m.mem, (uint8_t) op->Operand1(), (uintptr_t) op->Operand2());
      break;
    }
    if (m.mem->HasWriteError()) {
      return Err::OutOfMemory();
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
    ASSERT(target_pos > (uint8_t *) 0, "Label not found");
    ASSERT(jump->IsAny({Instruction::JZ, Instruction::JNZ}), "Invalid op code in jump list");
    if (jump->Is(Instruction::JZ)) {
      PatchJumpZero(*m.mem, code_pos, (uintptr_t) (target_pos - code_pos));
    } else if (jump->Is(Instruction::JNZ)) {
      PatchJumpNonZero(*m.mem, code_pos, (uintptr_t) (target_pos - code_pos));
    }
  }
  EmitExit(*m.mem);
  if (m.mem->HasWriteError()) {
    return Err::OutOfMemory();
  }
  Err err = m.mem->MakeExecutable();
  if (!err.IsOk()) {
    return err;
  }
  m.entry = reinterpret_cast<CodeEntry>(entry);
  return Err::Ok();
}
