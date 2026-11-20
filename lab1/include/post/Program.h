#pragma once

#include <cstddef>
#include <iosfwd>
#include <map>

#include "post/Rule.h"

/// @file
/// @brief Программа машины Поста — набор пронумерованных правил.

namespace post {

/**
 * @brief Программа: правила, отсортированные по номерам.
 *
 * Текстовый формат — по одному правилу в строке. Пустые строки и строки,
 * начинающиеся с `#`, пропускаются.
 */
class Program {
public:
    /**
     * @brief Добавляет правило.
     * @throws std::invalid_argument если правило с таким номером уже есть
     */
    void addRule(const Rule& rule);

    /// Удаляет правило с номером @p number; возвращает true, если такое правило было.
    bool removeRule(int number);

    /// Есть ли правило с номером @p number.
    bool hasRule(int number) const;

    /**
     * @brief Просмотр правила по номеру.
     * @throws std::out_of_range если такого правила нет
     */
    const Rule& rule(int number) const;

    /// Просмотр всех правил в порядке возрастания номеров.
    const std::map<int, Rule>& rules() const;

    /// Количество правил.
    std::size_t size() const;

    /// В программе нет правил.
    bool isEmpty() const;

    /**
     * @brief Наименьший номер правила (с него начинается выполнение).
     * @throws std::logic_error если программа пуста
     */
    int firstRuleNumber() const;

    /**
     * @brief Загружает программу из потока до его конца, заменяя текущие правила.
     * @throws std::invalid_argument если строка не является правилом или номера повторяются
     *         (текущие правила при этом не меняются)
     */
    void load(std::istream& input);

    /// Программы равны, если содержат одинаковые правила.
    bool operator==(const Program& other) const;

    /// Программы различаются.
    bool operator!=(const Program& other) const;

private:
    std::map<int, Rule> rules_;
};

/// Записывает программу: по одному правилу в строке.
std::ostream& operator<<(std::ostream& output, const Program& program);

/**
 * @brief Читает программу из потока до его конца.
 *
 * При ошибке устанавливает failbit, а @p program остаётся без изменений.
 */
std::istream& operator>>(std::istream& input, Program& program);

}  // namespace post
