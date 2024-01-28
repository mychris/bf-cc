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
  EXPECT_TRUE(stream.Begin().LookingAt({
        Instruction::JUMP_ZERO,
        Instruction::IMUL_CELL,
        Instruction::SET_CELL,
        Instruction::JUMP_NON_ZERO
      }));
  EXPECT_EQ(1, (stream.Begin() + 1)->Operand1());
  EXPECT_EQ(1, (stream.Begin() + 1)->Operand2());
  EXPECT_EQ(0, (stream.Begin() + 2)->Operand1());
  EXPECT_EQ(0, (stream.Begin() + 2)->Operand2());
}

TEST(TestOptMultiplyLoop, multByThreeSingle) {
  OperationStream stream = std::get<OperationStream>(parse("[- > +++ <]"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptMultiplyLoop(stream);
  EXPECT_TRUE(stream.Begin().LookingAt({
        Instruction::JUMP_ZERO,
        Instruction::IMUL_CELL,
        Instruction::SET_CELL,
        Instruction::JUMP_NON_ZERO
      }));
  EXPECT_EQ(3, (stream.Begin() + 1)->Operand1());
  EXPECT_EQ(1, (stream.Begin() + 1)->Operand2());
  EXPECT_EQ(0, (stream.Begin() + 2)->Operand1());
  EXPECT_EQ(0, (stream.Begin() + 2)->Operand2());
}

TEST(TestOptMultiplyLoop, multByThreeAndFive) {
  OperationStream stream = std::get<OperationStream>(parse("[- > +++ > +++++ < <]"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptMultiplyLoop(stream);
  EXPECT_TRUE(stream.Begin().LookingAt({
        Instruction::JUMP_ZERO,
        Instruction::IMUL_CELL,
        Instruction::IMUL_CELL,
        Instruction::SET_CELL,
        Instruction::JUMP_NON_ZERO
      }));
  EXPECT_EQ(3, (stream.Begin() + 1)->Operand1());
  EXPECT_EQ(1, (stream.Begin() + 1)->Operand2());
  EXPECT_EQ(5, (stream.Begin() + 2)->Operand1());
  EXPECT_EQ(2, (stream.Begin() + 2)->Operand2());
  EXPECT_EQ(0, (stream.Begin() + 3)->Operand1());
  EXPECT_EQ(0, (stream.Begin() + 3)->Operand2());
}

TEST(TestOptMultiplyLoop, multBetweenOther) {
  OperationStream stream = std::get<OperationStream>(parse("+[- > +++ > +++++ < <]+"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptMultiplyLoop(stream);
  EXPECT_TRUE(stream.Begin().LookingAt({
        Instruction::INCR_CELL,
        Instruction::JUMP_ZERO,
        Instruction::IMUL_CELL,
        Instruction::IMUL_CELL,
        Instruction::SET_CELL,
        Instruction::JUMP_NON_ZERO,
        Instruction::INCR_CELL
      }));
  EXPECT_EQ(3, (stream.Begin() + 2)->Operand1());
  EXPECT_EQ(1, (stream.Begin() + 2)->Operand2());
  EXPECT_EQ(5, (stream.Begin() + 3)->Operand1());
  EXPECT_EQ(2, (stream.Begin() + 3)->Operand2());
  EXPECT_EQ(0, (stream.Begin() + 4)->Operand1());
  EXPECT_EQ(0, (stream.Begin() + 4)->Operand2());
}

TEST(TestOptMultiplyLoop, twoMultiplications) {
  OperationStream stream = std::get<OperationStream>(parse("+[- > +++ <][- > +++ <]"));
  OptFusionOp(stream);
  OptDelayPtr(stream);
  OptMultiplyLoop(stream);
  ASSERT_TRUE(stream.Begin().LookingAt({
      Instruction::INCR_CELL,
      Instruction::JUMP_ZERO,
      Instruction::IMUL_CELL,
      Instruction::SET_CELL,
      Instruction::JUMP_NON_ZERO,
      Instruction::JUMP_ZERO,
      Instruction::IMUL_CELL,
      Instruction::SET_CELL,
      Instruction::JUMP_NON_ZERO,
  }));
  EXPECT_EQ(3, (stream.Begin() + 2)->Operand1());
  EXPECT_EQ(1, (stream.Begin() + 2)->Operand2());
  EXPECT_EQ(0, (stream.Begin() + 3)->Operand1());
  EXPECT_EQ(0, (stream.Begin() + 3)->Operand2());
  EXPECT_EQ(3, (stream.Begin() + 6)->Operand1());
  EXPECT_EQ(1, (stream.Begin() + 6)->Operand2());
  EXPECT_EQ(0, (stream.Begin() + 7)->Operand1());
  EXPECT_EQ(0, (stream.Begin() + 7)->Operand2());
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
