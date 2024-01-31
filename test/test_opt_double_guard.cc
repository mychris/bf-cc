// SPDX-License-Identifier: MIT License
#include "gtest/gtest.h"
#include "instr.h"
#include "optimize.h"
#include "parse.h"

TEST(TestOptDoubleGuard, emptyStream) {
  OperationStream stream = OperationStream::Create();
  OptDoubleGuard(stream);
  EXPECT_EQ(nullptr, stream.First());
  EXPECT_EQ(nullptr, stream.Last());
}

TEST(TestDoubleGuard, nopStream) {
  OperationStream stream = OperationStream::Create();
  stream.Prepend(Instruction::NOP);
  stream.Prepend(Instruction::NOP);
  stream.Prepend(Instruction::NOP);
  OptDoubleGuard(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_EQ(Instruction::NOP, instr->OpCode());
    ++count;
  }
  EXPECT_EQ(3, count);
}

TEST(TestDoubleGuard, nestedLoop) {
  OperationStream stream = std::get<OperationStream>(parse("[[+]]"));
  OptDoubleGuard(stream);
  ASSERT_TRUE(stream.Begin().LookingAt(
      {Instruction::JZ, Instruction::LABEL, Instruction::INCR_CELL, Instruction::JNZ, Instruction::LABEL}));
}

TEST(TestDoubleGuard, doubleEntry) {
  OperationStream stream = std::get<OperationStream>(parse("[[+]+]"));
  OptDoubleGuard(stream);
  ASSERT_TRUE(stream.Begin().LookingAt({Instruction::JZ,
                                        Instruction::LABEL,
                                        Instruction::LABEL,
                                        Instruction::INCR_CELL,
                                        Instruction::JNZ,
                                        Instruction::INCR_CELL,
                                        Instruction::JNZ,
                                        Instruction::LABEL}));
}

TEST(TestDoubleGuard, doubleExit) {
  OperationStream stream = std::get<OperationStream>(parse("[+[+]]"));
  OptDoubleGuard(stream);
  ASSERT_TRUE(stream.Begin().LookingAt({Instruction::JZ,
                                        Instruction::INCR_CELL,
                                        Instruction::JZ,
                                        Instruction::LABEL,
                                        Instruction::INCR_CELL,
                                        Instruction::JNZ,
                                        Instruction::LABEL,
                                        Instruction::LABEL}));
}
