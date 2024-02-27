// SPDX-License-Identifier: MIT License
#ifndef BF_CC_ASSEMBLER_H
#define BF_CC_ASSEMBLER_H 1

#include "error.h"
#include "instr.h"
#include "mem.h"

void EmitEntry(CodeArea &);
void EmitExit(CodeArea &);

void EmitNop(CodeArea &);

void EmitIncrCell(CodeArea &, uint8_t, intptr_t);
void EmitDecrCell(CodeArea &, uint8_t, intptr_t);

void EmitImullCell(CodeArea &, uint8_t, intptr_t);
void EmitDmullCell(CodeArea &, uint8_t, intptr_t);

void EmitSetCell(CodeArea &, uint8_t, intptr_t);

void EmitIncrPtr(CodeArea &, intptr_t);
void EmitDecrPtr(CodeArea &, intptr_t);

void EmitRead(CodeArea &, EOFMode);
void EmitWrite(CodeArea &);

void EmitJumpZero(CodeArea &);
void PatchJumpZero(CodeArea &, uint8_t *, uintptr_t);
void EmitJumpNonZero(CodeArea &);
void PatchJumpNonZero(CodeArea &, uint8_t *, uintptr_t);

void EmitFindCellHigh(CodeArea &, uint8_t, uintptr_t);
void EmitFindCellLow(CodeArea &, uint8_t, uintptr_t);

#endif /* BF_CC_ASSEMBLER_H */
