#pragma once

#include <iosfwd>
#include <optional>
#include <string>

#include "post/PostMachine.h"

/// @file
/// @brief Консольное меню для работы с машиной Поста.

namespace post {

/**
 * @brief Интерактивное меню: просмотр и правка правил и ленты, пошаговое и полное выполнение.
 *
 * Меню отделено от PostMachine и пользуется только её публичными методами.
 * Ошибки машины (например, обращение к несуществующему правилу) выводятся сообщением и не прерывают работу.
 */
class PostMachineMenu {
public:
    /**
     * @brief Создаёт меню.
     * @param input поток команд
     * @param output поток для вывода
     * @param machine машина, с которой работает меню
     * @param logEnabled выводить ли состояние после каждого правила при полном выполнении
     */
    PostMachineMenu(std::istream& input, std::ostream& output, PostMachine machine, bool logEnabled);

    /// Показывает меню и обрабатывает команды, пока не выбран выход или не закончился ввод.
    void run();

    /// Машина, с которой работает меню.
    const PostMachine& machine() const;

private:
    /// Пункты меню.
    enum class Item {
        Exit = 0,
        ShowState = 1,
        ListRules = 2,
        AddRule = 3,
        RemoveRule = 4,
        SetCell = 5,
        Step = 6,
        Run = 7,
        Reset = 8
    };

    /// Указатель на метод-обработчик пункта меню.
    using Action = void (PostMachineMenu::*)();

    Action findAction(int choice) const;
    bool processChoice();
    bool execute(int choice);
    void showState();
    void listRules();
    void addRule();
    void removeRule();
    void setCell();
    void step();
    void runProgram();
    void resetMachine();
    std::optional<long long> readNumber(const std::string& prompt);
    std::string readLine(const std::string& prompt);

    std::istream& input_;
    std::ostream& output_;
    PostMachine machine_;
    bool logEnabled_;
};

}  // namespace post
