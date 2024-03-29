// SPDX-License-Identifier: MIT License
#include "gtest/gtest.h"
#include "instr.h"
#include "optimize.h"
#include "parse.h"

TEST(TestOptCommentLoop, emptyStream) {
  OperationStream stream = OperationStream::Create();
  OptCommentLoop(stream);
  EXPECT_EQ(nullptr, stream.First());
  EXPECT_EQ(nullptr, stream.Last());
}

TEST(TestOptCommentLoop, nopStream) {
  OperationStream stream = OperationStream::Create();
  stream.Prepend(Instruction::NOP);
  stream.Prepend(Instruction::NOP);
  stream.Prepend(Instruction::NOP);
  OptCommentLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_EQ(Instruction::NOP, instr->OpCode());
    ++count;
  }
  EXPECT_EQ(3, count);
}

TEST(TestOptCommentLoop, noCommentLoop) {
  OperationStream stream = std::get<OperationStream>(Parse("++++++++++."));
  OptCommentLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    (void) instr;
    ++count;
  }
  EXPECT_EQ(11, count);
}

TEST(TestOptCommentLoop, commentLoopOnly) {
  OperationStream stream = std::get<OperationStream>(Parse("[here is a comment with, some. operations++]"));
  OptCommentLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    (void) instr;
    ++count;
  }
  EXPECT_EQ(0, count);
}

TEST(TestOptCommentLoop, commentLoopWithNopsOnly) {
  OperationStream stream = std::get<OperationStream>(Parse("[here is a comment with, some. operations++]"));
  OptCommentLoop(stream);
  stream.Prepend(Instruction::NOP);
  stream.Prepend(Instruction::NOP);
  size_t count = 0;
  for (auto *instr : stream) {
    EXPECT_EQ(instr->OpCode(), Instruction::NOP);
    ++count;
  }
  EXPECT_EQ(2, count);
}

TEST(TestOptCommentLoop, commentLoop) {
  OperationStream stream = std::get<OperationStream>(Parse("[here is a comment with, some. operations++]++++++++++."));
  OptCommentLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    (void) instr;
    ++count;
  }
  EXPECT_EQ(11, count);
}

TEST(TestOptCommentLoop, commentLoopWithNops) {
  OperationStream stream = std::get<OperationStream>(Parse("[here is a comment with, some. operations++]++++++++++."));
  stream.Prepend(Instruction::NOP);
  stream.Prepend(Instruction::NOP);
  OptCommentLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    (void) instr;
    ++count;
  }
  EXPECT_EQ(13, count);
}

TEST(TestOptCommentLoop, multipleCommentLoops) {
  OperationStream stream = std::get<OperationStream>(Parse("[++][--]++++++++++."));
  OptCommentLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    (void) instr;
    ++count;
  }
  EXPECT_EQ(11, count);
}

TEST(TestOptCommentLoop, multipleCommentLoopsWithNops) {
  OperationStream stream = std::get<OperationStream>(Parse("[++][--]++++++++++."));
  stream.Prepend(Instruction::NOP);
  stream.Prepend(Instruction::NOP);
  OptCommentLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    (void) instr;
    ++count;
  }
  EXPECT_EQ(13, count);
}
