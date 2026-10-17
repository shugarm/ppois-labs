#pragma once

#include <cstdint>
#include <istream>
#include <stdexcept>
#include <string>

/// @file
/// @brief Небольшие вспомогательные функции для разбора текста.

namespace post {

/// Тип координаты на бесконечной ленте.
using Position = std::int64_t;

/// Символ, с которого начинается строка-комментарий во входных файлах.
constexpr char kCommentMarker = '#';

/// Удаляет пробельные символы в начале и в конце строки.
inline std::string trim(const std::string& text) {
    const char* const whitespace = " \t\r\n";
    const std::size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return std::string();
    }
    const std::size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

/// Строка пуста или является комментарием (такие строки пропускаются при чтении).
inline bool isIgnorableLine(const std::string& trimmedLine) {
    return trimmedLine.empty() || trimmedLine.front() == kCommentMarker;
}

/**
 * @brief Преобразует строку в целое число.
 * @throws std::invalid_argument если строка не является целым числом
 */
inline Position parseInteger(const std::string& text) {
    const std::string trimmed = trim(text);
    std::size_t consumed = 0;
    Position value = 0;
    try {
        value = std::stoll(trimmed, &consumed);
    } catch (const std::logic_error&) {
        throw std::invalid_argument("Expected an integer, got: '" + trimmed + "'");
    }
    if (consumed != trimmed.size()) {
        throw std::invalid_argument("Expected an integer, got: '" + trimmed + "'");
    }
    return value;
}

/**
 * @brief Убирает failbit, возникший только из-за конца потока.
 *
 * После цикла `while (std::getline(...))` поток всегда находится в состоянии
 * failbit|eofbit. Если причина только в конце данных, чтение считается успешным.
 */
inline void clearEndOfFileFailure(std::istream& input) {
    if (input.eof() && !input.bad()) {
        input.clear(std::ios::eofbit);
    }
}

}  // namespace post
