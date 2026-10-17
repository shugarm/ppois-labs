#pragma once

#include <cstddef>
#include <iosfwd>
#include <optional>
#include <string>

#include "fifteen/FifteenPuzzle.h"

/// @file
/// @brief Консольное меню для игры «Пятнашки».

namespace fifteen {

/**
 * @brief Консольный интерфейс игры «Пятнашки».
 *
 * Класс отделён от логики игры: он только вызывает публичные методы FifteenPuzzle.
 * Потоки ввода и вывода передаются снаружи, поэтому меню можно проверять без консоли.
 */
class FifteenMenu {
public:
    /**
     * @brief Создаёт меню.
     * @param input поток, из которого читаются команды
     * @param output поток, в который выводится меню и поле
     * @param puzzle начальная головоломка (по умолчанию случайная 4x4)
     */
    FifteenMenu(std::istream& input, std::ostream& output, FifteenPuzzle puzzle = FifteenPuzzle());

    /// Показывает меню и обрабатывает команды, пока не выбран выход или не закончился ввод.
    void run();

    /// Текущая головоломка.
    const FifteenPuzzle& puzzle() const;

    /// Количество сделанных ходов в текущей партии.
    std::size_t moveCount() const;

private:
    /// Пункты меню.
    enum class Item {
        Exit = 0,
        NewGame = 1,
        ShowBoard = 2,
        MoveTile = 3,
        CheckSolved = 4,
        Save = 5,
        Load = 6
    };

    /// Указатель на метод-обработчик пункта меню.
    using Action = void (FifteenMenu::*)();

    void printMenu() const;
    void printBoard();
    Action findAction(int choice) const;
    bool processChoice();
    bool execute(int choice);
    void startNewGame();
    void moveTile();
    void checkSolved();
    void saveToFile();
    void loadFromFile();
    std::optional<int> readNumber(const std::string& prompt);
    std::string readLine(const std::string& prompt);

    std::istream& input_;
    std::ostream& output_;
    FifteenPuzzle puzzle_;
    std::size_t moveCount_;
};

}  // namespace fifteen
