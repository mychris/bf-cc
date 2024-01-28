// SPDX-License-Identifier: MIT License
#include "gtest/gtest.h"
#include "instr.h"
#include "interp.h"
#include "mem.h"
#include "parse.h"

TEST(TestInterpreter, emptyStream) {
  OperationStream stream = OperationStream::Create();
  Heap heap = std::get<Heap>(Heap::Create(128));
  Interpreter::Create().Run(heap, stream, EOFMode::KEEP);
  EXPECT_EQ(0, heap.DataPointer());
  for (int i = 0; i < 128; ++i) {
    EXPECT_EQ(0, heap.GetCell(i));
  }
}

TEST(TestInterpreter, nopStream) {
  OperationStream stream = OperationStream::Create();
  stream.Append(Instruction::NOP);
  stream.Append(Instruction::NOP);
  stream.Append(Instruction::NOP);
  stream.Append(Instruction::NOP);
  Heap heap = std::get<Heap>(Heap::Create(128));
  Interpreter::Create().Run(heap, stream, EOFMode::KEEP);
  EXPECT_EQ(0, heap.DataPointer());
  for (int i = 0; i < 128; ++i) {
    EXPECT_EQ(0, heap.GetCell(i));
  }
}

TEST(TestInterpreter, singleCellOperations) {
  OperationStream stream = std::get<OperationStream>(parse("++--+++---+"));
  Heap heap = std::get<Heap>(Heap::Create(128));
  Interpreter::Create().Run(heap, stream, EOFMode::KEEP);
  EXPECT_EQ(0, heap.DataPointer());
  EXPECT_EQ(1, heap.GetCell(0));
  for (int i = 1; i < 128; ++i) {
    EXPECT_EQ(0, heap.GetCell(i));
  }
}

TEST(TestInterpreter, twoCellOperations) {
  OperationStream stream = std::get<OperationStream>(parse("++--+>++----+"));
  Heap heap = std::get<Heap>(Heap::Create(128));
  Interpreter::Create().Run(heap, stream, EOFMode::KEEP);
  EXPECT_EQ(1, heap.DataPointer());
  EXPECT_EQ(1, heap.GetCell(-1));
  EXPECT_EQ((uint8_t) -1, heap.GetCell(0));
  for (int i = 2; i < 128; ++i) {
    EXPECT_EQ(0, heap.GetCell(i));
  }
}
