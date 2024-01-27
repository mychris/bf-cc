#include "gtest/gtest.h"

#include "instr.h"
#include "optimize.h"

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
    (void) instr;
    ++count;
  }
  EXPECT_EQ(3, count);
}
