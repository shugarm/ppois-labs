#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "fifteen/FifteenMenu.h"

using fifteen::FifteenMenu;
using fifteen::FifteenPuzzle;

namespace {

const std::vector<int> kSolved3 = {1, 2, 3, 4, 5, 6, 7, 8, 0};
const std::vector<int> kOneMoveFromSolved3 = {1, 2, 3, 4, 5, 6, 7, 0, 8};

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

std::string runMenu(const std::string& commands, FifteenPuzzle puzzle = FifteenPuzzle(3, kSolved3)) {
    std::istringstream input(commands);
    std::ostringstream output;
    FifteenMenu menu(input, output, std::move(puzzle));
    menu.run();
    return output.str();
}

std::string temporaryFile(const std::string& name) {
    return (std::filesystem::temp_directory_path() / name).string();
}

}  // namespace

TEST(FifteenMenu, ExitStopsTheLoop) {
    const std::string output = runMenu("0\n");
    EXPECT_TRUE(contains(output, "Пятнашки"));
}

TEST(FifteenMenu, EndOfInputStopsTheLoop) {
    EXPECT_NO_THROW(runMenu(""));
    EXPECT_NO_THROW(runMenu("2"));
}

TEST(FifteenMenu, InvalidChoicesAreReported) {
    const std::string output = runMenu("abc\n1 2\n99999999999\n7\n-1\n0\n");
    EXPECT_TRUE(contains(output, "Введите номер пункта меню"));
    EXPECT_TRUE(contains(output, "Такого пункта в меню нет"));
}

TEST(FifteenMenu, ShowBoardPrintsEmptyCellAsDot) {
    const std::string output = runMenu("2\n0\n");
    EXPECT_TRUE(contains(output, "  7  8  ."));
}

TEST(FifteenMenu, NewGameCreatesBoardOfRequestedSize) {
    std::istringstream input("1\n5\n0\n");
    std::ostringstream output;
    FifteenMenu menu(input, output, FifteenPuzzle(3, kSolved3));
    menu.run();
    EXPECT_EQ(menu.puzzle().size(), 5u);
    EXPECT_EQ(menu.moveCount(), 0u);
}

TEST(FifteenMenu, NewGameRejectsBadSize) {
    const std::string output = runMenu("1\n1\n1\nabc\n1\n11\n0\n");
    EXPECT_TRUE(contains(output, "Недопустимый размер поля"));
}

TEST(FifteenMenu, MovingTilesCountsMovesAndDetectsVictory) {
    std::istringstream input("3\n8\n0\n");
    std::ostringstream output;
    FifteenMenu menu(input, output, FifteenPuzzle(3, kOneMoveFromSolved3));
    menu.run();
    EXPECT_EQ(menu.moveCount(), 1u);
    EXPECT_TRUE(menu.puzzle().isSolved());
    EXPECT_TRUE(contains(output.str(), "Поздравляем"));
}

TEST(FifteenMenu, MovingTileWithoutVictoryDoesNotCongratulate) {
    const std::string output = runMenu("3\n8\n0\n");
    EXPECT_FALSE(contains(output, "Поздравляем"));
}

TEST(FifteenMenu, ImpossibleMoveAndBadNumberAreReported) {
    const std::string output = runMenu("3\n1\n3\nabc\n0\n");
    EXPECT_TRUE(contains(output, "нельзя сдвинуть"));
    EXPECT_TRUE(contains(output, "Нужно ввести число"));
}

TEST(FifteenMenu, CheckSolvedReportsBothOutcomes) {
    EXPECT_TRUE(contains(runMenu("4\n0\n"), "головоломка собрана"));
    EXPECT_TRUE(contains(runMenu("4\n0\n", FifteenPuzzle(3, kOneMoveFromSolved3)), "Пока не собрана"));
}

TEST(FifteenMenu, SaveAndLoadRestoreThePuzzle) {
    const std::string path = temporaryFile("fifteen_menu_save_test.txt");
    runMenu("5\n" + path + "\n0\n", FifteenPuzzle(3, kOneMoveFromSolved3));
    std::istringstream input("6\n" + path + "\n0\n");
    std::ostringstream output;
    FifteenMenu menu(input, output, FifteenPuzzle(3, kSolved3));
    menu.run();
    std::remove(path.c_str());
    EXPECT_TRUE(menu.puzzle() == FifteenPuzzle(3, kOneMoveFromSolved3));
    EXPECT_TRUE(contains(output.str(), "Игра загружена"));
}

TEST(FifteenMenu, FileProblemsAreReported) {
    const std::string missing = temporaryFile("fifteen_menu_missing_dir/none.txt");
    EXPECT_TRUE(contains(runMenu("5\n" + missing + "\n0\n"), "Не удалось открыть файл"));
    EXPECT_TRUE(contains(runMenu("6\n" + missing + "\n0\n"), "Не удалось открыть файл"));
}

TEST(FifteenMenu, LoadingBrokenFileIsReported) {
    const std::string path = temporaryFile("fifteen_menu_broken_test.txt");
    {
        std::ofstream file(path);
        file << "3\n1 1 1\n";
    }
    const std::string output = runMenu("6\n" + path + "\n0\n");
    std::remove(path.c_str());
    EXPECT_TRUE(contains(output, "некорректную головоломку"));
}

TEST(FifteenMenu, SavedFileContainsPuzzleDescription) {
    const std::string path = temporaryFile("fifteen_menu_content_test.txt");
    runMenu("5\n" + path + "\n0\n");
    std::ifstream file(path);
    std::string firstLine;
    std::getline(file, firstLine);
    file.close();
    std::remove(path.c_str());
    EXPECT_EQ(firstLine, "3");
}
