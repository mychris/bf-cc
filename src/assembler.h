// SPDX-License-Identifier: MIT License
#ifndef BF_CC_ASSEMBLER_H
#define BF_CC_ASSEMBLER_H 1

#include "error.h"
#include "instr.h"
#include "mem.h"

Err EmitEntry(CodeArea &);
Err EmitExit(CodeArea &);

Err EmitNop(CodeArea &);

Err EmitIncrCell(CodeArea &, uint8_t, intptr_t);
Err EmitDecrCell(CodeArea &, uint8_t, intptr_t);

Err EmitImullCell(CodeArea &, uint8_t, intptr_t);
Err EmitDmullCell(CodeArea &, uint8_t, intptr_t);

Err EmitSetCell(CodeArea &, uint8_t, intptr_t);

Err EmitIncrPtr(CodeArea &, intptr_t);
Err EmitDecrPtr(CodeArea &, intptr_t);

Err EmitRead(CodeArea &, EOFMode);
Err EmitWrite(CodeArea &);

Err EmitJumpZero(CodeArea &);
Err PatchJumpZero(CodeArea &, uint8_t *, uintptr_t);
Err EmitJumpNonZero(CodeArea &);
Err PatchJumpNonZero(CodeArea &, uint8_t *, uintptr_t);

Err EmitFindCellHigh(CodeArea &, uint8_t, uintptr_t);
Err EmitFindCellLow(CodeArea &, uint8_t, uintptr_t);

#endif /* BF_CC_ASSEMBLER_H */
