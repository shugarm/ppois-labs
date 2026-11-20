#include "fifteen/FifteenMenu.h"

#include <fstream>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <utility>
#include <vector>

namespace fifteen {

namespace {

constexpr int kBoardCellWidth = 3;
constexpr const char* kEmptyTileSymbol = ".";

constexpr const char* kMenuText =
    "\n=== Пятнашки ===\n"
    "1. Новая игра\n"
    "2. Показать поле\n"
    "3. Передвинуть плитку\n"
    "4. Проверить, собрана ли головоломка\n"
    "5. Сохранить игру в файл\n"
    "6. Загрузить игру из файла\n"
    "0. Выход\n";

constexpr const char* kChoicePrompt = "Ваш выбор: ";
constexpr const char* kInvalidChoiceMessage = "Введите номер пункта меню.\n";
constexpr const char* kUnknownItemMessage = "Такого пункта в меню нет.\n";
constexpr const char* kSizePromptLabel = "Размер поля";
constexpr const char* kInvalidSizeMessage = "Недопустимый размер поля.\n";
constexpr const char* kTilePrompt = "Номер плитки: ";
constexpr const char* kInvalidNumberMessage = "Нужно ввести число.\n";
constexpr const char* kCannotMoveMessage = "Эту плитку сейчас нельзя сдвинуть.\n";
constexpr const char* kMovesLabel = "Ходов: ";
constexpr const char* kSolvedMessage = "Поздравляем, головоломка собрана!\n";
constexpr const char* kNotSolvedMessage = "Пока не собрана.\n";
constexpr const char* kFilePrompt = "Путь к файлу: ";
constexpr const char* kSavedMessage = "Игра сохранена.\n";
constexpr const char* kLoadedMessage = "Игра загружена.\n";
constexpr const char* kCannotOpenMessage = "Не удалось открыть файл.\n";
constexpr const char* kInvalidFileMessage = "Файл содержит некорректную головоломку.\n";

std::string makeSizePrompt() {
    std::ostringstream prompt;
    prompt << kSizePromptLabel << " (" << FifteenPuzzle::kMinSize << "-" << FifteenPuzzle::kMaxSize << "): ";
    return prompt.str();
}

bool isSizeInRange(int size) {
    return size >= static_cast<int>(FifteenPuzzle::kMinSize) && size <= static_cast<int>(FifteenPuzzle::kMaxSize);
}

}  // namespace

FifteenMenu::FifteenMenu(std::istream& input, std::ostream& output, FifteenPuzzle puzzle)
    : input_(input), output_(output), puzzle_(std::move(puzzle)), moveCount_(0) {}

const FifteenPuzzle& FifteenMenu::puzzle() const {
    return puzzle_;
}

std::size_t FifteenMenu::moveCount() const {
    return moveCount_;
}

void FifteenMenu::run() {
    bool running = true;
    while (running && input_) {
        printMenu();
        running = processChoice();
    }
}

void FifteenMenu::printMenu() const {
    output_ << kMenuText;
}

void FifteenMenu::printBoard() {
    output_ << '\n';
    for (std::size_t row = 0; row < puzzle_.size(); ++row) {
        for (std::size_t col = 0; col < puzzle_.size(); ++col) {
            const int tile = puzzle_[row][col];
            output_ << std::setw(kBoardCellWidth);
            if (tile == FifteenPuzzle::kEmptyTile) {
                output_ << kEmptyTileSymbol;
            } else {
                output_ << tile;
            }
        }
        output_ << '\n';
    }
}

bool FifteenMenu::processChoice() {
    const std::optional<int> choice = readNumber(kChoicePrompt);
    if (!input_) {
        return false;
    }
    if (!choice) {
        output_ << kInvalidChoiceMessage;
        return true;
    }
    return execute(*choice);
}

FifteenMenu::Action FifteenMenu::findAction(int choice) const {
    static const std::vector<std::pair<Item, Action>> actions = {
        {Item::NewGame, &FifteenMenu::startNewGame},     {Item::ShowBoard, &FifteenMenu::printBoard},
        {Item::MoveTile, &FifteenMenu::moveTile},        {Item::CheckSolved, &FifteenMenu::checkSolved},
        {Item::Save, &FifteenMenu::saveToFile},          {Item::Load, &FifteenMenu::loadFromFile}};
    for (const auto& entry : actions) {
        if (static_cast<int>(entry.first) == choice) {
            return entry.second;
        }
    }
    return nullptr;
}

bool FifteenMenu::execute(int choice) {
    if (choice == static_cast<int>(Item::Exit)) {
        return false;
    }
    const Action action = findAction(choice);
    if (action == nullptr) {
        output_ << kUnknownItemMessage;
        return true;
    }
    (this->*action)();
    return true;
}

void FifteenMenu::startNewGame() {
    const std::optional<int> size = readNumber(makeSizePrompt());
    if (!size || !isSizeInRange(*size)) {
        output_ << kInvalidSizeMessage;
        return;
    }
    puzzle_ = FifteenPuzzle(static_cast<std::size_t>(*size));
    moveCount_ = 0;
    printBoard();
}

void FifteenMenu::moveTile() {
    const std::optional<int> tile = readNumber(kTilePrompt);
    if (!tile) {
        output_ << kInvalidNumberMessage;
        return;
    }
    if (!puzzle_.moveTileByValue(*tile)) {
        output_ << kCannotMoveMessage;
        return;
    }
    ++moveCount_;
    printBoard();
    if (puzzle_.isSolved()) {
        output_ << kSolvedMessage << kMovesLabel << moveCount_ << '\n';
    }
}

void FifteenMenu::checkSolved() {
    output_ << (puzzle_.isSolved() ? kSolvedMessage : kNotSolvedMessage);
}

void FifteenMenu::saveToFile() {
    const std::string path = readLine(kFilePrompt);
    std::ofstream file(path);
    if (!file) {
        output_ << kCannotOpenMessage;
        return;
    }
    file << puzzle_;
    output_ << kSavedMessage;
}

void FifteenMenu::loadFromFile() {
    const std::string path = readLine(kFilePrompt);
    std::ifstream file(path);
    if (!file) {
        output_ << kCannotOpenMessage;
        return;
    }
    FifteenPuzzle loaded;
    if (!(file >> loaded)) {
        output_ << kInvalidFileMessage;
        return;
    }
    puzzle_ = loaded;
    moveCount_ = 0;
    output_ << kLoadedMessage;
    printBoard();
}

std::optional<int> FifteenMenu::readNumber(const std::string& prompt) {
    const std::string line = readLine(prompt);
    std::istringstream parser(line);
    int value = 0;
    char trailing = 0;
    if (!input_ || !(parser >> value) || (parser >> trailing)) {
        return std::nullopt;
    }
    return value;
}

std::string FifteenMenu::readLine(const std::string& prompt) {
    output_ << prompt;
    std::string line;
    std::getline(input_, line);
    return line;
}

}  // namespace fifteen
