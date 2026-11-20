#include "fifteen/FifteenPuzzle.h"

#include <algorithm>
#include <iomanip>
#include <istream>
#include <numeric>
#include <ostream>
#include <random>
#include <stdexcept>

namespace fifteen {

namespace {

constexpr std::size_t kShuffleMovesPerCell = 100;
constexpr std::size_t kCellOutputWidth = 2;
constexpr std::size_t kParityDivisor = 2;
constexpr std::size_t kAdjacentDistance = 1;
constexpr int kFirstTile = 1;

bool isOdd(std::size_t value) {
    return value % kParityDivisor != 0;
}

std::size_t absoluteDifference(std::size_t first, std::size_t second) {
    return first > second ? first - second : second - first;
}

bool isValidSize(std::size_t size) {
    return size >= FifteenPuzzle::kMinSize && size <= FifteenPuzzle::kMaxSize;
}

std::vector<int> makeSolvedTiles(std::size_t size) {
    std::vector<int> tiles(size * size);
    std::iota(tiles.begin(), tiles.end(), kFirstTile);
    tiles.back() = FifteenPuzzle::kEmptyTile;
    return tiles;
}

bool isPermutationFromZero(std::vector<int> tiles) {
    std::sort(tiles.begin(), tiles.end());
    std::vector<int> expected(tiles.size());
    std::iota(expected.begin(), expected.end(), FifteenPuzzle::kEmptyTile);
    return tiles == expected;
}

std::size_t countSmallerTilesAfter(const std::vector<int>& tiles, std::size_t current) {
    const int currentValue = tiles[current];
    const auto isSmaller = [currentValue](int other) {
        return other != FifteenPuzzle::kEmptyTile && other < currentValue;
    };
    const auto firstAfter = tiles.begin() + static_cast<std::ptrdiff_t>(current + 1);
    return static_cast<std::size_t>(std::count_if(firstAfter, tiles.end(), isSmaller));
}

std::size_t countInversions(const std::vector<int>& tiles) {
    std::size_t inversions = 0;
    for (std::size_t current = 0; current < tiles.size(); ++current) {
        if (tiles[current] != FifteenPuzzle::kEmptyTile) {
            inversions += countSmallerTilesAfter(tiles, current);
        }
    }
    return inversions;
}

bool readTiles(std::istream& input, std::size_t size, std::vector<int>& tiles) {
    if (!isValidSize(size)) {
        input.setstate(std::ios::failbit);
        return false;
    }
    tiles.resize(size * size);
    for (int& tile : tiles) {
        if (!(input >> tile)) {
            return false;
        }
    }
    return true;
}

void randomWalk(FifteenPuzzle& puzzle, std::mt19937& generator) {
    const std::size_t moves = puzzle.size() * puzzle.size() * kShuffleMovesPerCell;
    for (std::size_t moveIndex = 0; moveIndex < moves; ++moveIndex) {
        const std::vector<Position> candidates = puzzle.movableTiles();
        std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
        puzzle.moveTile(candidates[pick(generator)]);
    }
}

}  // namespace

bool operator==(const Position& left, const Position& right) {
    return left.row == right.row && left.col == right.col;
}

bool operator!=(const Position& left, const Position& right) {
    return !(left == right);
}

FifteenPuzzle::RowView::RowView(const FifteenPuzzle& puzzle, std::size_t row) : puzzle_(puzzle), row_(row) {}

int FifteenPuzzle::RowView::operator[](std::size_t col) const {
    if (col >= puzzle_.size_) {
        throw std::out_of_range("Column index is outside of the board");
    }
    return puzzle_.tiles_[row_ * puzzle_.size_ + col];
}

FifteenPuzzle::FifteenPuzzle(std::size_t size) : size_(size) {
    if (!isValidSize(size)) {
        throw std::invalid_argument("Board size is out of the allowed range");
    }
    tiles_ = makeSolvedTiles(size);
    shuffle();
}

FifteenPuzzle::FifteenPuzzle(std::size_t size, const std::vector<int>& tiles) : size_(size), tiles_(tiles) {
    if (!isValidSize(size)) {
        throw std::invalid_argument("Board size is out of the allowed range");
    }
    if (tiles.size() != size * size || !isPermutationFromZero(tiles)) {
        throw std::invalid_argument("Tiles must be a permutation of 0..size*size-1");
    }
}

std::size_t FifteenPuzzle::size() const {
    return size_;
}

FifteenPuzzle::RowView FifteenPuzzle::operator[](std::size_t row) const {
    if (row >= size_) {
        throw std::out_of_range("Row index is outside of the board");
    }
    return RowView(*this, row);
}

std::size_t FifteenPuzzle::indexOf(Position position) const {
    return position.row * size_ + position.col;
}

Position FifteenPuzzle::positionOf(std::size_t index) const {
    return Position{index / size_, index % size_};
}

std::size_t FifteenPuzzle::emptyIndex() const {
    const auto found = std::find(tiles_.begin(), tiles_.end(), kEmptyTile);
    return static_cast<std::size_t>(found - tiles_.begin());
}

Position FifteenPuzzle::emptyPosition() const {
    return positionOf(emptyIndex());
}

std::vector<Position> FifteenPuzzle::movableTiles() const {
    std::vector<Position> result;
    const Position empty = emptyPosition();
    if (empty.row > 0) {
        result.push_back({empty.row - 1, empty.col});
    }
    if (empty.row + 1 < size_) {
        result.push_back({empty.row + 1, empty.col});
    }
    if (empty.col > 0) {
        result.push_back({empty.row, empty.col - 1});
    }
    if (empty.col + 1 < size_) {
        result.push_back({empty.row, empty.col + 1});
    }
    return result;
}

bool FifteenPuzzle::canMove(Position position) const {
    if (position.row >= size_ || position.col >= size_) {
        return false;
    }
    const Position empty = emptyPosition();
    const std::size_t rowDistance = absoluteDifference(position.row, empty.row);
    const std::size_t colDistance = absoluteDifference(position.col, empty.col);
    const std::size_t distance = rowDistance + colDistance;
    return distance == kAdjacentDistance;
}

bool FifteenPuzzle::moveTile(Position position) {
    if (!canMove(position)) {
        return false;
    }
    std::swap(tiles_[indexOf(position)], tiles_[emptyIndex()]);
    return true;
}

bool FifteenPuzzle::moveTileByValue(int value) {
    const auto found = std::find(tiles_.begin(), tiles_.end(), value);
    if (found == tiles_.end() || value == kEmptyTile) {
        return false;
    }
    return moveTile(positionOf(static_cast<std::size_t>(found - tiles_.begin())));
}

void FifteenPuzzle::shuffle() {
    std::random_device device;
    shuffle(device());
}

void FifteenPuzzle::shuffle(unsigned seed) {
    std::mt19937 generator(seed);
    do {
        tiles_ = makeSolvedTiles(size_);
        randomWalk(*this, generator);
    } while (isSolved());
}

bool FifteenPuzzle::isSolved() const {
    return tiles_ == makeSolvedTiles(size_);
}

bool FifteenPuzzle::isSolvable() const {
    const std::size_t inversions = countInversions(tiles_);
    if (isOdd(size_)) {
        return !isOdd(inversions);
    }
    const std::size_t emptyRowFromBottom = size_ - emptyPosition().row;
    return isOdd(inversions + emptyRowFromBottom);
}

bool FifteenPuzzle::operator==(const FifteenPuzzle& other) const {
    return size_ == other.size_ && tiles_ == other.tiles_;
}

bool FifteenPuzzle::operator!=(const FifteenPuzzle& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& output, const FifteenPuzzle& puzzle) {
    output << puzzle.size() << '\n';
    for (std::size_t row = 0; row < puzzle.size(); ++row) {
        for (std::size_t col = 0; col < puzzle.size(); ++col) {
            output << (col > 0 ? " " : "") << std::setw(kCellOutputWidth) << puzzle[row][col];
        }
        output << '\n';
    }
    return output;
}

std::istream& operator>>(std::istream& input, FifteenPuzzle& puzzle) {
    std::size_t size = 0;
    std::vector<int> tiles;
    if (!(input >> size) || !readTiles(input, size, tiles)) {
        return input;
    }
    try {
        puzzle = FifteenPuzzle(size, tiles);
    } catch (const std::invalid_argument&) {
        input.setstate(std::ios::failbit);
    }
    return input;
}

}  // namespace fifteen
