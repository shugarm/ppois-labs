#pragma once

#include <iosfwd>
#include <string>
#include <vector>

/// @file
/// @brief Консольное приложение для запуска программ машины Поста.

namespace post {

/// Код возврата: всё выполнено успешно.
constexpr int kExitSuccess = 0;
/// Код возврата: ошибка чтения файла или выполнения программы.
constexpr int kExitFailure = 1;
/// Код возврата: неверные аргументы командной строки.
constexpr int kExitUsageError = 2;

/// Параметры командной строки.
struct Options {
    std::string path;          ///< Путь к файлу с состоянием машины и программой.
    bool log = false;          ///< Флаг `-log`: выводить состояние после каждого правила.
    bool interactive = false;  ///< Флаг `-i`: открыть интерактивное меню вместо автоматического запуска.
};

/**
 * @brief Разбирает аргументы командной строки (без имени программы).
 * @throws std::invalid_argument при неизвестном флаге, нескольких путях или отсутствии пути без флага `-i`
 */
Options parseArguments(const std::vector<std::string>& args);

/**
 * @brief Запускает приложение.
 *
 * Без флага `-i` читает файл, выполняет программу и выводит итоговое состояние машины.
 * С флагом `-log` после каждого правила выводится текущее состояние.
 * С флагом `-i` после загрузки открывается интерактивное меню.
 *
 * @param args аргументы командной строки без имени программы
 * @param input поток ввода (для меню)
 * @param output поток вывода
 * @param errors поток сообщений об ошибках
 * @return код возврата процесса
 */
int runApplication(const std::vector<std::string>& args, std::istream& input, std::ostream& output,
                   std::ostream& errors);

}  // namespace post
