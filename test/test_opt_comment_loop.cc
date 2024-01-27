#include "gtest/gtest.h"

#include "instr.h"
#include "optimize.h"

TEST(testOptCommentLoop, emptyStream) {
  Operation::Stream stream = Operation::Stream::Create();
  OptCommentLoop(stream);
  EXPECT_EQ(nullptr, stream.First());
  EXPECT_EQ(nullptr, stream.Last());
}

TEST(testOptCommentLoop, nopStream) {
  Operation::Stream stream = Operation::Stream::Create();
  stream.Prepend(Instruction::NOP);
  stream.Prepend(Instruction::NOP);
  stream.Prepend(Instruction::NOP);
  OptCommentLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    ++count;
  }
  EXPECT_EQ(3, count);
}
