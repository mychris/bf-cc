// SPDX-License-Identifier: MIT License
#include "gtest/gtest.h"
#include "instr.h"
#include "optimize.h"
#include "parse.h"

TEST(TestOptFusionOp, emptyStream) {
  OperationStream stream = OperationStream::Create();
  OptFusionOp(stream);
  EXPECT_EQ(nullptr, stream.First());
  EXPECT_EQ(nullptr, stream.Last());
}

TEST(TestOptFusionOp, nopStream) {
  OperationStream stream = OperationStream::Create();
  stream.Prepend(Instruction::NOP);
  stream.Prepend(Instruction::NOP);
  stream.Prepend(Instruction::NOP);
  OptFusionOp(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_EQ(Instruction::NOP, instr->OpCode());
    ++count;
  }
  EXPECT_EQ(3, count);
}

TEST(TestOptFusionOp, onlyIncrement) {
  OperationStream stream = std::get<OperationStream>(parse("++++"));
  OptFusionOp(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_EQ(Instruction::INCR_CELL, instr->OpCode());
    EXPECT_EQ(4, instr->Operand1());
    ++count;
  }
  EXPECT_EQ(1, count);
}

TEST(TestOptFusionOp, onlyDecrement) {
  OperationStream stream = std::get<OperationStream>(parse("----"));
  OptFusionOp(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_EQ(Instruction::DECR_CELL, instr->OpCode());
    EXPECT_EQ(4, instr->Operand1());
    ++count;
  }
  EXPECT_EQ(1, count);
}

TEST(TestOptFusionOp, onlyLeftPointer) {
  OperationStream stream = std::get<OperationStream>(parse("<<<<"));
  OptFusionOp(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_EQ(Instruction::DECR_PTR, instr->OpCode());
    EXPECT_EQ(4, instr->Operand1());
    ++count;
  }
  EXPECT_EQ(1, count);
}

TEST(TestOptFusionOp, onlyRightPointer) {
  OperationStream stream = std::get<OperationStream>(parse(">>>>"));
  OptFusionOp(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_EQ(Instruction::INCR_PTR, instr->OpCode());
    EXPECT_EQ(4, instr->Operand1());
    ++count;
  }
  EXPECT_EQ(1, count);
}

TEST(TestOptFusionOp, onlyIncrementDecrement) {
  OperationStream stream = std::get<OperationStream>(parse("++++----++++----"));
  OptFusionOp(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_EQ(4, instr->Operand1());
    ++count;
  }
  EXPECT_EQ(4, count);
}

TEST(TestOptFusionOp, fuseInLoop) {
  OperationStream stream = std::get<OperationStream>(parse("[>++++>++++<<<<----]"));
  OptFusionOp(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    if (Instruction::INCR_CELL == instr->OpCode() || Instruction::DECR_CELL == instr->OpCode()
        || Instruction::DECR_PTR == instr->OpCode()) {
      EXPECT_EQ(4, instr->Operand1());
    }
    ++count;
  }
  EXPECT_EQ(10, count);
}

TEST(TestOptFusionOp, doNotFuseReads) {
  OperationStream stream = std::get<OperationStream>(parse(",,"));
  OptFusionOp(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    (void) instr;
    ++count;
  }
  EXPECT_EQ(2, count);
}

TEST(TestOptFusionOp, doNotFuseWrites) {
  OperationStream stream = std::get<OperationStream>(parse(".."));
  OptFusionOp(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    (void) instr;
    ++count;
  }
  EXPECT_EQ(2, count);
}

TEST(TestOptFusionOp, doNotFuseAfterDelayIncrement) {
  OperationStream stream = std::get<OperationStream>(parse("[++++>++++]"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptFusionOp(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    if (Instruction::INCR_CELL == instr->OpCode()) {
      EXPECT_EQ(4, instr->Operand1());
    }
    ++count;
  }
  EXPECT_EQ(7, count);
}
