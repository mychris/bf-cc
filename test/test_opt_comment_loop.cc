#include "gtest/gtest.h"

#include "instr.h"
#include "optimize.h"

TEST(testOptCommentLoop, emptyStream) {
  Instr::Stream stream = Instr::Stream::Create();
  OptCommentLoop(stream);
  EXPECT_EQ(nullptr, stream.First());
  EXPECT_EQ(nullptr, stream.Last());
}

TEST(testOptCommentLoop, nopStream) {
  Instr::Stream stream = Instr::Stream::Create();
  stream.Prepend(InstrCode::NOP);
  stream.Prepend(InstrCode::NOP);
  stream.Prepend(InstrCode::NOP);
  OptCommentLoop(stream);
  size_t count = 0;
  for (auto *instr : stream) {
    ++count;
  }
  EXPECT_EQ(3, count);
}
