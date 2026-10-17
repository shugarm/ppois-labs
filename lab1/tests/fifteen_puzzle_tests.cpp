#include <gtest/gtest.h>

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "fifteen/FifteenPuzzle.h"

using fifteen::FifteenPuzzle;
using fifteen::Position;

namespace {

constexpr unsigned kSeed = 12345;
constexpr unsigned kOtherSeed = 54321;
constexpr unsigned kSeedCount = 20;
constexpr int kMissingTile = 99;

const std::vector<int> kSolved3 = {1, 2, 3, 4, 5, 6, 7, 8, 0};
const std::vector<int> kOneMoveFromSolved3 = {1, 2, 3, 4, 5, 6, 7, 0, 8};
const std::vector<int> kEmptyInCenter3 = {1, 2, 3, 4, 0, 5, 6, 7, 8};

std::vector<int> solvedTiles(std::size_t size) {
    std::vector<int> tiles(size * size);
    for (std::size_t index = 0; index + 1 < tiles.size(); ++index) {
        tiles[index] = static_cast<int>(index) + 1;
    }
    tiles.back() = 0;
    return tiles;
}

}  // namespace

TEST(FifteenPuzzle, ConstructorRejectsSizeOutsideAllowedRange) {
    EXPECT_THROW(static_cast<void>(FifteenPuzzle(FifteenPuzzle::kMinSize - 1)), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(FifteenPuzzle(FifteenPuzzle::kMaxSize + 1)), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(FifteenPuzzle(0, std::vector<int>())), std::invalid_argument);
}

TEST(FifteenPuzzle, DefaultConstructorCreatesShuffledSolvableClassicBoard) {
    const FifteenPuzzle puzzle;
    EXPECT_EQ(puzzle.size(), FifteenPuzzle::kDefaultSize);
    EXPECT_FALSE(puzzle.isSolved());
    EXPECT_TRUE(puzzle.isSolvable());
}

TEST(FifteenPuzzle, ShuffleAlwaysGivesSolvableAndNotSolvedBoardOfAnySize) {
    for (std::size_t size = FifteenPuzzle::kMinSize; size <= FifteenPuzzle::kMaxSize; ++size) {
        FifteenPuzzle puzzle(size, solvedTiles(size));
        for (unsigned seed = 0; seed < kSeedCount; ++seed) {
            puzzle.shuffle(seed);
            EXPECT_TRUE(puzzle.isSolvable());
            EXPECT_FALSE(puzzle.isSolved());
        }
    }
}

TEST(FifteenPuzzle, ShuffleWithoutSeedGivesSolvableBoard) {
    FifteenPuzzle puzzle(3, kSolved3);
    puzzle.shuffle();
    EXPECT_TRUE(puzzle.isSolvable());
    EXPECT_FALSE(puzzle.isSolved());
}

TEST(FifteenPuzzle, ShuffleWithSameSeedIsDeterministic) {
    FifteenPuzzle first(4, solvedTiles(4));
    FifteenPuzzle second(4, solvedTiles(4));
    first.shuffle(kSeed);
    second.shuffle(kSeed);
    EXPECT_TRUE(first == second);
    second.shuffle(kOtherSeed);
    EXPECT_TRUE(first != second);
}

TEST(FifteenPuzzle, ConstructorWithTilesAcceptsPermutation) {
    const FifteenPuzzle puzzle(3, kEmptyInCenter3);
    EXPECT_EQ(puzzle.size(), 3u);
    EXPECT_EQ(puzzle[1][1], FifteenPuzzle::kEmptyTile);
}

TEST(FifteenPuzzle, ConstructorWithTilesRejectsInvalidArrangements) {
    EXPECT_THROW(FifteenPuzzle(3, std::vector<int>{1, 2, 3}), std::invalid_argument);
    EXPECT_THROW(FifteenPuzzle(3, std::vector<int>{1, 1, 3, 4, 5, 6, 7, 8, 0}), std::invalid_argument);
    EXPECT_THROW(FifteenPuzzle(3, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}), std::invalid_argument);
    EXPECT_THROW(FifteenPuzzle(3, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, -1}), std::invalid_argument);
}

TEST(FifteenPuzzle, IndexOperatorReturnsCellValues) {
    const FifteenPuzzle puzzle(3, kSolved3);
    EXPECT_EQ(puzzle[0][0], 1);
    EXPECT_EQ(puzzle[1][2], 6);
    EXPECT_EQ(puzzle[2][2], FifteenPuzzle::kEmptyTile);
}

TEST(FifteenPuzzle, IndexOperatorChecksBounds) {
    const FifteenPuzzle puzzle(3, kSolved3);
    EXPECT_THROW(static_cast<void>(puzzle[3]), std::out_of_range);
    EXPECT_THROW(static_cast<void>(puzzle[0][3]), std::out_of_range);
}

TEST(FifteenPuzzle, EmptyPositionIsFound) {
    EXPECT_TRUE(FifteenPuzzle(3, kSolved3).emptyPosition() == (Position{2, 2}));
    EXPECT_TRUE(FifteenPuzzle(3, kEmptyInCenter3).emptyPosition() == (Position{1, 1}));
}

TEST(FifteenPuzzle, MovableTilesAreNeighboursOfEmptyCell) {
    EXPECT_EQ(FifteenPuzzle(3, kSolved3).movableTiles().size(), 2u);
    EXPECT_EQ(FifteenPuzzle(3, kEmptyInCenter3).movableTiles().size(), 4u);
    EXPECT_EQ(FifteenPuzzle(3, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 0, 8}).movableTiles().size(), 3u);
    EXPECT_EQ(FifteenPuzzle(3, std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8}).movableTiles().size(), 2u);
}

TEST(FifteenPuzzle, CanMoveOnlyForNeighboursOfEmptyCell) {
    const FifteenPuzzle puzzle(3, kSolved3);
    EXPECT_TRUE(puzzle.canMove({1, 2}));
    EXPECT_TRUE(puzzle.canMove({2, 1}));
    EXPECT_FALSE(puzzle.canMove({1, 1}));
    EXPECT_FALSE(puzzle.canMove({0, 0}));
    EXPECT_FALSE(puzzle.canMove({2, 2}));
    EXPECT_FALSE(puzzle.canMove({3, 2}));
    EXPECT_FALSE(puzzle.canMove({2, 3}));
}

TEST(FifteenPuzzle, MoveTileSwapsTileWithEmptyCell) {
    FifteenPuzzle puzzle(3, kSolved3);
    EXPECT_TRUE(puzzle.moveTile({2, 1}));
    EXPECT_EQ(puzzle[2][2], 8);
    EXPECT_EQ(puzzle[2][1], FifteenPuzzle::kEmptyTile);
    EXPECT_TRUE(puzzle.emptyPosition() == (Position{2, 1}));
}

TEST(FifteenPuzzle, MoveTileRefusesImpossibleMoves) {
    FifteenPuzzle puzzle(3, kSolved3);
    const FifteenPuzzle before = puzzle;
    EXPECT_FALSE(puzzle.moveTile({0, 0}));
    EXPECT_FALSE(puzzle.moveTile({5, 5}));
    EXPECT_TRUE(puzzle == before);
}

TEST(FifteenPuzzle, MoveTileByValueMovesNeighbourTile) {
    FifteenPuzzle puzzle(3, kOneMoveFromSolved3);
    EXPECT_TRUE(puzzle.moveTileByValue(8));
    EXPECT_TRUE(puzzle.isSolved());
}

TEST(FifteenPuzzle, MoveTileByValueRefusesUnknownOrDistantTiles) {
    FifteenPuzzle puzzle(3, kSolved3);
    EXPECT_FALSE(puzzle.moveTileByValue(1));
    EXPECT_FALSE(puzzle.moveTileByValue(kMissingTile));
    EXPECT_FALSE(puzzle.moveTileByValue(-1));
    EXPECT_FALSE(puzzle.moveTileByValue(FifteenPuzzle::kEmptyTile));
}

TEST(FifteenPuzzle, IsSolvedDetectsSolvedArrangementOnly) {
    FifteenPuzzle puzzle(3, kSolved3);
    EXPECT_TRUE(puzzle.isSolved());
    puzzle.moveTile({2, 1});
    EXPECT_FALSE(puzzle.isSolved());
    puzzle.moveTile({2, 2});
    EXPECT_TRUE(puzzle.isSolved());
}

TEST(FifteenPuzzle, IsSolvableForKnownArrangements) {
    EXPECT_TRUE(FifteenPuzzle(3, kSolved3).isSolvable());
    EXPECT_TRUE(FifteenPuzzle(2, std::vector<int>{1, 2, 3, 0}).isSolvable());
    EXPECT_FALSE(FifteenPuzzle(2, std::vector<int>{2, 1, 3, 0}).isSolvable());
    EXPECT_FALSE(FifteenPuzzle(3, std::vector<int>{2, 1, 3, 4, 5, 6, 7, 8, 0}).isSolvable());
    std::vector<int> swappedFourteenFifteen = solvedTiles(4);
    std::swap(swappedFourteenFifteen[13], swappedFourteenFifteen[14]);
    EXPECT_FALSE(FifteenPuzzle(4, swappedFourteenFifteen).isSolvable());
    EXPECT_TRUE(FifteenPuzzle(4, solvedTiles(4)).isSolvable());
}

TEST(FifteenPuzzle, EqualityComparesSizeAndTiles) {
    const FifteenPuzzle first(3, kSolved3);
    const FifteenPuzzle second(3, kSolved3);
    const FifteenPuzzle third(3, kOneMoveFromSolved3);
    const FifteenPuzzle fourth(2, std::vector<int>{1, 2, 3, 0});
    EXPECT_TRUE(first == second);
    EXPECT_FALSE(first != second);
    EXPECT_TRUE(first != third);
    EXPECT_TRUE(first != fourth);
}

TEST(FifteenPuzzle, CopiesAreIndependent) {
    FifteenPuzzle original(3, kSolved3);
    FifteenPuzzle copy(original);
    FifteenPuzzle assigned(2, std::vector<int>{1, 2, 3, 0});
    assigned = original;
    copy.moveTile({2, 1});
    EXPECT_TRUE(original.isSolved());
    EXPECT_FALSE(copy.isSolved());
    EXPECT_TRUE(assigned == original);
}

TEST(FifteenPuzzle, PositionComparison) {
    EXPECT_TRUE((Position{1, 2}) == (Position{1, 2}));
    EXPECT_TRUE((Position{1, 2}) != (Position{2, 1}));
}

TEST(FifteenPuzzle, OutputUsesDocumentedFormat) {
    std::ostringstream output;
    output << FifteenPuzzle(3, kSolved3);
    EXPECT_EQ(output.str(), "3\n 1  2  3\n 4  5  6\n 7  8  0\n");
}

TEST(FifteenPuzzle, StreamRoundTripPreservesPuzzle) {
    for (std::size_t size = FifteenPuzzle::kMinSize; size <= FifteenPuzzle::kMaxSize; ++size) {
        const FifteenPuzzle original(size);
        std::stringstream stream;
        stream << original;
        FifteenPuzzle loaded(2, std::vector<int>{1, 2, 3, 0});
        stream >> loaded;
        EXPECT_FALSE(stream.fail());
        EXPECT_TRUE(loaded == original);
    }
}

TEST(FifteenPuzzle, InputRejectsMalformedData) {
    const std::vector<std::string> badInputs = {
        "", "abc", "1\n0\n", "11\n", "3\n1 2 3 4 5 6 7 8", "3\n1 1 3 4 5 6 7 8 0", "3\n1 2 3 4 5 6 7 8 x"};
    for (const std::string& text : badInputs) {
        FifteenPuzzle puzzle(3, kSolved3);
        std::istringstream input(text);
        input >> puzzle;
        EXPECT_TRUE(input.fail());
        EXPECT_TRUE(puzzle == FifteenPuzzle(3, kSolved3));
    }
}
