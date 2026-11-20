#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>

/// @file
/// @brief Правило (строка программы) машины Поста.

namespace post {

/// Тип команды машины Поста.
enum class CommandType {
    Mark,       ///< `V` — поставить метку и перейти к правилу first.
    Erase,      ///< `X` — стереть метку и перейти к правилу first.
    MoveLeft,   ///< `L` — сдвинуть каретку влево и перейти к правилу first.
    MoveRight,  ///< `R` — сдвинуть каретку вправо и перейти к правилу first.
    Check,      ///< `?` — если ячейка помечена, перейти к правилу first, иначе к правилу second.
    Stop        ///< `!` — остановить машину.
};

/// Символ, которым команда записывается в тексте программы.
char commandSymbol(CommandType type);

/**
 * @brief Тип команды по её символу.
 * @throws std::invalid_argument если символ не обозначает команду
 */
CommandType commandTypeFromSymbol(char symbol);

/// Сколько номеров правил для перехода указывается у команды.
std::size_t targetCount(CommandType type);

/**
 * @brief Одно правило программы: номер, команда и номера правил для перехода.
 *
 * Текстовый формат: `<номер> <команда> [переходы]`, например `1 V 2`, `2 ? 3 1`, `3 !`.
 */
class Rule {
public:
    /// Наименьший допустимый номер правила.
    static constexpr int kMinRuleNumber = 1;
    /// Значение для неиспользуемого перехода.
    static constexpr int kNoTarget = -1;

    /// Создаёт правило `1 !` (остановка).
    Rule();

    /**
     * @brief Создаёт правило.
     * @param number номер правила (не меньше kMinRuleNumber)
     * @param type команда
     * @param firstTarget первый переход (нужен всем командам, кроме Stop)
     * @param secondTarget второй переход (нужен только команде Check)
     * @throws std::invalid_argument если номер или нужные переходы некорректны
     */
    Rule(int number, CommandType type, int firstTarget = kNoTarget, int secondTarget = kNoTarget);

    /**
     * @brief Разбирает правило из строки.
     * @throws std::invalid_argument если строка не является правилом
     */
    static Rule parse(const std::string& line);

    /// Номер правила.
    int number() const;

    /// Команда правила.
    CommandType type() const;

    /// Первый переход (kNoTarget, если не используется).
    int firstTarget() const;

    /// Второй переход (kNoTarget, если не используется).
    int secondTarget() const;

    /// Правила равны, если совпадают номер, команда и переходы.
    bool operator==(const Rule& other) const;

    /// Правила различаются.
    bool operator!=(const Rule& other) const;

private:
    int number_;
    CommandType type_;
    int firstTarget_;
    int secondTarget_;
};

/// Записывает правило в текстовом формате.
std::ostream& operator<<(std::ostream& output, const Rule& rule);

/**
 * @brief Читает правило из потока.
 *
 * При некорректном содержимом устанавливает failbit, а @p rule остаётся без изменений.
 */
std::istream& operator>>(std::istream& input, Rule& rule);

}  // namespace post
