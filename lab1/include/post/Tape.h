#pragma once

#include <iosfwd>
#include <set>
#include <string>

#include "post/TextUtils.h"

/// @file
/// @brief Бесконечная лента машины Поста.

/// Классы машины Поста.
namespace post {

/// Символ помеченной ячейки в текстовом представлении ленты.
constexpr char kMarkedChar = '1';
/// Символ пустой ячейки в текстовом представлении ленты.
constexpr char kEmptyChar = '0';
/// Разделитель координаты первой ячейки и содержимого ленты в тексте.
constexpr char kOriginSeparator = ':';

/**
 * @brief Бесконечная в обе стороны лента, каждая ячейка которой помечена или пуста.
 *
 * Хранятся только координаты помеченных ячеек, поэтому лента не ограничена по длине.
 *
 * Текстовый формат: `<начало>:<ячейки>`, например `-2:1011`. Начало (координата первой
 * записанной ячейки) можно опустить, тогда оно равно 0. Символ `1` — метка, `0` — пустая
 * ячейка, пробелы игнорируются.
 */
class Tape {
public:
    /// Создаёт пустую ленту.
    Tape() = default;

    /**
     * @brief Создаёт ленту из текстового представления.
     * @param text строка вида `1101` или `-2:1101`
     * @throws std::invalid_argument если строка содержит недопустимые символы или начало не является числом
     */
    explicit Tape(const std::string& text);

    /// Помечена ли ячейка с данной координатой.
    bool isMarked(Position position) const;

    /// Значение ячейки (то же, что isMarked), удобно писать `tape[position]`.
    bool operator[](Position position) const;

    /// Ставит метку в ячейку (если метка уже есть, ничего не меняется).
    void mark(Position position);

    /// Стирает метку из ячейки (если ячейка пуста, ничего не меняется).
    void erase(Position position);

    /// Задаёт значение ячейки: true — метка, false — пусто.
    void set(Position position, bool marked);

    /// Нет ни одной метки.
    bool isEmpty() const;

    /// Количество меток на ленте.
    std::size_t markedCount() const;

    /**
     * @brief Координата самой левой метки.
     * @throws std::logic_error если лента пуста
     */
    Position leftmost() const;

    /**
     * @brief Координата самой правой метки.
     * @throws std::logic_error если лента пуста
     */
    Position rightmost() const;

    /// Строка из символов `0`/`1` для ячеек с координатами от @p from до @p to включительно.
    std::string render(Position from, Position to) const;

    /// Ленты равны, если помечены одни и те же ячейки.
    bool operator==(const Tape& other) const;

    /// Ленты различаются.
    bool operator!=(const Tape& other) const;

private:
    void loadCells(const std::string& cells, Position origin);

    std::set<Position> markedCells_;
};

/// Записывает ленту в формате `<начало>:<ячейки>` (пустая лента — `0:`).
std::ostream& operator<<(std::ostream& output, const Tape& tape);

/**
 * @brief Читает ленту из одной строки потока.
 *
 * При некорректном содержимом устанавливает failbit, а @p tape остаётся без изменений.
 */
std::istream& operator>>(std::istream& input, Tape& tape);

}  // namespace post
