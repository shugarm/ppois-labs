#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

/// @file
/// @brief Настройка кодировки консоли для вывода русского текста.

/// Вспомогательные функции для работы с консолью.
namespace console {

/**
 * @brief Включает UTF-8 в консоли Windows, чтобы русский текст отображался правильно.
 *
 * В остальных системах функция ничего не делает: там консоль уже работает в UTF-8.
 */
inline void enableUtf8() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

}  // namespace console
