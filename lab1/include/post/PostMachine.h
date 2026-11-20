#pragma once

#include <cstddef>
#include <iosfwd>
#include <stdexcept>
#include <string>

#include "post/Carriage.h"
#include "post/Program.h"
#include "post/Tape.h"

/// @file
/// @brief Машина Поста.

namespace post {

/// Ошибка выполнения программы машины Поста.
class PostMachineError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/**
 * @brief Машина Поста: лента, каретка и программа.
 *
 * Машина выполняет правила, начиная с правила с наименьшим номером. Команды:
 * `V` (поставить метку), `X` (стереть метку), `L`/`R` (сдвиг каретки), `?` (условный переход)
 * и `!` (остановка). Постановка метки на помеченную ячейку и стирание метки с пустой ячейки
 * считаются ошибкой выполнения (PostMachineError).
 *
 * Формат файла (строки с `#` и пустые строки пропускаются):
 * @code
 * tape: 1101
 * carriage: 0
 * program:
 * 1 R 2
 * 2 ? 1 3
 * 3 !
 * @endcode
 */
class PostMachine {
public:
    /// Ограничение на число шагов в run() по умолчанию (защита от бесконечных циклов).
    static constexpr std::size_t kDefaultStepLimit = 1000000;

    /// Создаёт машину с пустой лентой, кареткой в позиции 0 и пустой программой.
    PostMachine() = default;

    /// Создаёт машину из готовых частей.
    PostMachine(Tape tape, Carriage carriage, Program program);

    /**
     * @brief Загружает программу из потока (правила по одному в строке), заменяя прежнюю.
     * @throws std::invalid_argument при ошибке в тексте программы
     */
    void loadProgram(std::istream& input);

    /**
     * @brief Загружает состояние ленты из одной строки потока (формат Tape).
     * @throws std::invalid_argument если строка не является лентой
     */
    void loadTape(std::istream& input);

    /**
     * @brief Загружает состояние машины целиком из потока (формат файла см. в описании класса).
     * @throws std::invalid_argument при ошибке формата
     */
    void load(std::istream& input);

    /**
     * @brief Добавляет правило в программу.
     * @throws std::invalid_argument если правило с таким номером уже есть
     */
    void addRule(const Rule& rule);

    /// Удаляет правило с номером @p number; возвращает true, если оно существовало.
    bool removeRule(int number);

    /// Программа машины (для просмотра правил).
    const Program& program() const;

    /// Лента машины.
    const Tape& tape() const;

    /// Каретка машины.
    const Carriage& carriage() const;

    /// Значение ячейки ленты.
    bool cell(Position position) const;

    /// Задаёт значение ячейки ленты: true — метка, false — пусто.
    void setCell(Position position, bool marked);

    /// Ставит каретку над ячейкой с указанной координатой.
    void moveCarriageTo(Position position);

    /// Номер правила, которое будет выполнено следующим (Rule::kNoTarget, если программа пуста).
    int currentRuleNumber() const;

    /// Машина остановилась командой `!`.
    bool isHalted() const;

    /// Возвращает машину к началу программы (первое правило, машина не остановлена). Лента не меняется.
    void reset();

    /**
     * @brief Выполняет одно правило.
     * @return true, если правило выполнено; false, если машина уже остановлена
     * @throws PostMachineError если программа пуста, нужного правила нет или команду нельзя выполнить
     */
    bool step();

    /**
     * @brief Выполняет программу до остановки.
     * @param stepLimit максимальное число шагов
     * @param log если не nullptr, после каждого правила в поток выводится состояние машины
     * @return количество выполненных шагов
     * @throws PostMachineError при ошибке выполнения или превышении @p stepLimit
     */
    std::size_t run(std::size_t stepLimit = kDefaultStepLimit, std::ostream* log = nullptr);

    /// Краткое описание состояния: положение каретки и лента (ячейка под кареткой в скобках).
    std::string describe() const;

    /// Машины равны, если равны лента, каретка, программа и состояние выполнения.
    bool operator==(const PostMachine& other) const;

    /// Машины различаются.
    bool operator!=(const PostMachine& other) const;

private:
    const Rule& currentRule() const;
    void applyCommand(const Rule& rule);
    int nextRuleNumber(const Rule& rule) const;
    void markCurrentCell();
    void eraseCurrentCell();
    std::string renderTapeWindow() const;

    Tape tape_;
    Carriage carriage_;
    Program program_;
    int currentRuleNumber_ = Rule::kNoTarget;
    bool halted_ = false;
};

/// Записывает состояние машины в формате файла (см. описание класса).
std::ostream& operator<<(std::ostream& output, const PostMachine& machine);

/**
 * @brief Читает состояние машины в формате файла.
 *
 * При ошибке устанавливает failbit, а @p machine остаётся без изменений.
 */
std::istream& operator>>(std::istream& input, PostMachine& machine);

}  // namespace post
