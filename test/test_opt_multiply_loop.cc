// SPDX-License-Identifier: MIT License
#include "gtest/gtest.h"
#include "instr.h"
#include "optimize.h"
#include "parse.h"

TEST(TestOptMultiplyLoop, emptyStream) {
  OperationStream stream = OperationStream::Create();
  OptMultiplyLoop(stream);
  EXPECT_EQ(nullptr, stream.First());
  EXPECT_EQ(nullptr, stream.Last());
}

TEST(TestOptMultiplyLoop, multByOneSingle) {
  OperationStream stream = std::get<OperationStream>(parse("[- > + <]"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptMultiplyLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_TRUE(instr->IsAny({Instruction::SET_CELL, Instruction::IMUL_CELL}));
    if (instr->Is(Instruction::SET_CELL)) {
      EXPECT_EQ(0, instr->Operand1());
      EXPECT_EQ(0, instr->Operand2());
    }
    if (instr->Is(Instruction::IMUL_CELL)) {
      EXPECT_EQ(1, instr->Operand1());
      EXPECT_EQ(1, instr->Operand2());
    }
    ++count;
  }
  EXPECT_EQ(2, count);
}

TEST(TestOptMultiplyLoop, multByThreeSingle) {
  OperationStream stream = std::get<OperationStream>(parse("[- > +++ <]"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptMultiplyLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_TRUE(instr->IsAny({Instruction::SET_CELL, Instruction::IMUL_CELL}));
    if (instr->Is(Instruction::SET_CELL)) {
      EXPECT_EQ(0, instr->Operand1());
      EXPECT_EQ(0, instr->Operand2());
    }
    if (instr->Is(Instruction::IMUL_CELL)) {
      EXPECT_EQ(3, instr->Operand1());
      EXPECT_EQ(1, instr->Operand2());
    }
    ++count;
  }
  EXPECT_EQ(2, count);
}

TEST(TestOptMultiplyLoop, multByThreeAndFive) {
  OperationStream stream = std::get<OperationStream>(parse("[- > +++ > +++++ < <]"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptMultiplyLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_TRUE(instr->IsAny({Instruction::SET_CELL, Instruction::IMUL_CELL}));
    if (instr->Is(Instruction::SET_CELL)) {
      EXPECT_EQ(0, instr->Operand1());
      EXPECT_EQ(0, instr->Operand2());
    }
    if (instr->Is(Instruction::IMUL_CELL)) {
      EXPECT_TRUE(3 == instr->Operand1() || 5 == instr->Operand1());
      EXPECT_TRUE(1 == instr->Operand2() || 2 == instr->Operand2());
    }
    ++count;
  }
  EXPECT_EQ(3, count);
}

TEST(TestOptMultiplyLoop, multBetweenOther) {
  OperationStream stream = std::get<OperationStream>(parse("+[- > +++ > +++++ < <]+"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptMultiplyLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_TRUE(instr->IsAny({Instruction::SET_CELL, Instruction::IMUL_CELL, Instruction::INCR_CELL}));
    if (instr->Is(Instruction::SET_CELL)) {
      EXPECT_EQ(0, instr->Operand1());
      EXPECT_EQ(0, instr->Operand2());
    }
    if (instr->Is(Instruction::IMUL_CELL)) {
      EXPECT_TRUE(3 == instr->Operand1() || 5 == instr->Operand1());
      EXPECT_TRUE(1 == instr->Operand2() || 2 == instr->Operand2());
    }
    if (instr->Is(Instruction::INCR_CELL)) {
      EXPECT_TRUE(1 == instr->Operand1());
    }
    ++count;
  }
  EXPECT_EQ(5, count);
}

TEST(TestOptMultiplyLoop, twoMultiplications) {
  OperationStream stream = std::get<OperationStream>(parse("+[- > +++ <][- > +++ <]"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptMultiplyLoop(stream);
  ASSERT_TRUE(stream.Begin().LookingAt({
      Instruction::INCR_CELL,
      Instruction::IMUL_CELL,
      Instruction::SET_CELL,
      Instruction::IMUL_CELL,
      Instruction::SET_CELL,
  }));
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_TRUE(instr->IsAny({Instruction::SET_CELL, Instruction::IMUL_CELL, Instruction::INCR_CELL}));
    if (instr->Is(Instruction::SET_CELL)) {
      EXPECT_EQ(0, instr->Operand1());
      EXPECT_EQ(0, instr->Operand2());
    }
    if (instr->Is(Instruction::IMUL_CELL)) {
      EXPECT_TRUE(3 == instr->Operand1());
      EXPECT_EQ(1, instr->Operand2());
    }
    if (instr->Is(Instruction::INCR_CELL)) {
      EXPECT_TRUE(1 == instr->Operand1());
    }
    ++count;
  }
  EXPECT_EQ(5, count);
}

TEST(TestOptMultiplyLoop, multWithWrite) {
  OperationStream stream = std::get<OperationStream>(parse("[- > +++ < .]"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptMultiplyLoop(stream);
  for (auto *instr : stream) {
    EXPECT_TRUE(!instr->Is(Instruction::IMUL_CELL));
  }
}

TEST(TestOptMultiplyLoop, multWithNestedLoop) {
  OperationStream stream = std::get<OperationStream>(parse("[- > +++ < [,]]"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptMultiplyLoop(stream);
  for (auto *instr : stream) {
    EXPECT_TRUE(!instr->Is(Instruction::IMUL_CELL));
  }
}
