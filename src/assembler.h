// SPDX-License-Identifier: MIT License
#ifndef BF_CC_ASSEMBLER_H
#define BF_CC_ASSEMBLER_H 1

#include "error.h"
#include "instr.h"
#include "mem.h"

Err EmitEntry(CodeArea &);
Err EmitExit(CodeArea &);

Err EmitNop(CodeArea &);

Err EmitIncrCell(CodeArea &mem, uint8_t amount, intptr_t offset);
Err EmitDecrCell(CodeArea &mem, uint8_t amount, intptr_t offset);

Err EmitImullCell(CodeArea &mem, uint8_t amount, intptr_t offset);
Err EmitDmullCell(CodeArea &mem, uint8_t amount, intptr_t offset);

Err EmitSetCell(CodeArea &mem, uint8_t amount, intptr_t offset);

Err EmitIncrPtr(CodeArea &mem, intptr_t amount);
Err EmitDecrPtr(CodeArea &mem, intptr_t amount);

Err EmitRead(CodeArea &mem, EOFMode eof_mode);
Err EmitWrite(CodeArea &mem);

Err EmitJumpZero(CodeArea &mem);
Err PatchJumpZero(CodeArea &mem, uint8_t *position, uintptr_t offset);
Err EmitJumpNonZero(CodeArea &mem);
Err PatchJumpNonZero(CodeArea &mem, uint8_t *position, uintptr_t offset);

Err EmitFindCellHigh(CodeArea &mem, uint8_t value, uintptr_t move_size);
Err EmitFindCellLow(CodeArea &mem, uint8_t value, uintptr_t move_size);

#endif /* BF_CC_ASSEMBLER_H */
