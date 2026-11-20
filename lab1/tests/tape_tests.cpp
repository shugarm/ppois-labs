#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

#include "post/Tape.h"

using post::Tape;

TEST(Tape, NewTapeIsEmpty) {
    const Tape tape;
    EXPECT_TRUE(tape.isEmpty());
    EXPECT_EQ(tape.markedCount(), 0u);
    EXPECT_FALSE(tape.isMarked(0));
}

TEST(Tape, MarkAndEraseChangeCells) {
    Tape tape;
    tape.mark(5);
    tape.mark(-3);
    tape.mark(5);
    EXPECT_TRUE(tape.isMarked(5));
    EXPECT_TRUE(tape[-3]);
    EXPECT_EQ(tape.markedCount(), 2u);
    tape.erase(5);
    tape.erase(100);
    EXPECT_FALSE(tape[5]);
    EXPECT_EQ(tape.markedCount(), 1u);
}

TEST(Tape, SetWritesRequestedValue) {
    Tape tape;
    tape.set(2, true);
    EXPECT_TRUE(tape[2]);
    tape.set(2, false);
    EXPECT_FALSE(tape[2]);
}

TEST(Tape, BoundsOfNonEmptyTape) {
    Tape tape;
    tape.mark(-4);
    tape.mark(7);
    EXPECT_EQ(tape.leftmost(), -4);
    EXPECT_EQ(tape.rightmost(), 7);
}

TEST(Tape, BoundsOfEmptyTapeAreAnError) {
    const Tape tape;
    EXPECT_THROW(static_cast<void>(tape.leftmost()), std::logic_error);
    EXPECT_THROW(static_cast<void>(tape.rightmost()), std::logic_error);
}

TEST(Tape, RenderShowsRequestedWindow) {
    const Tape tape("1011");
    EXPECT_EQ(tape.render(0, 3), "1011");
    EXPECT_EQ(tape.render(-1, 4), "010110");
    EXPECT_EQ(tape.render(3, 1), "");
}

TEST(Tape, ConstructorParsesCellsAndIgnoresSpaces) {
    const Tape tape("11 0 1");
    EXPECT_EQ(tape.markedCount(), 3u);
    EXPECT_TRUE(tape[0]);
    EXPECT_TRUE(tape[1]);
    EXPECT_FALSE(tape[2]);
    EXPECT_TRUE(tape[3]);
}

TEST(Tape, ConstructorParsesOrigin) {
    const Tape tape("-2: 101");
    EXPECT_TRUE(tape[-2]);
    EXPECT_FALSE(tape[-1]);
    EXPECT_TRUE(tape[0]);
}

TEST(Tape, ConstructorRejectsBadText) {
    EXPECT_THROW(Tape("10x1"), std::invalid_argument);
    EXPECT_THROW(Tape("x:101"), std::invalid_argument);
    EXPECT_THROW(Tape("1x:101"), std::invalid_argument);
    EXPECT_THROW(Tape(":101"), std::invalid_argument);
    EXPECT_THROW(Tape("99999999999999999999:1"), std::invalid_argument);
}

TEST(Tape, EqualityComparesMarkedCells) {
    EXPECT_TRUE(Tape("101") == Tape("1 0 1"));
    EXPECT_TRUE(Tape("101") != Tape("111"));
    EXPECT_TRUE(Tape("101") == Tape("0:101"));
    EXPECT_TRUE(Tape("001") == Tape("2:1"));
}

TEST(Tape, OutputWritesOriginAndCells) {
    std::ostringstream output;
    output << Tape("-2:101") << '|' << Tape();
    EXPECT_EQ(output.str(), "-2:101|0:");
}

TEST(Tape, InputReadsOneLine) {
    std::istringstream input("1101\n-1:11\n");
    Tape first;
    Tape second;
    input >> first >> second;
    EXPECT_FALSE(input.fail());
    EXPECT_TRUE(first == Tape("1101"));
    EXPECT_TRUE(second == Tape("-1:11"));
}

TEST(Tape, InputFailsOnBadOrMissingLine) {
    Tape tape("11");
    std::istringstream bad("12\n");
    bad >> tape;
    EXPECT_TRUE(bad.fail());
    EXPECT_TRUE(tape == Tape("11"));
    std::istringstream empty("");
    empty >> tape;
    EXPECT_TRUE(empty.fail());
}

TEST(Tape, StreamRoundTripKeepsPositions) {
    Tape original;
    original.mark(-5);
    original.mark(0);
    original.mark(9);
    std::stringstream stream;
    stream << original << '\n';
    Tape loaded;
    stream >> loaded;
    EXPECT_TRUE(loaded == original);
}
