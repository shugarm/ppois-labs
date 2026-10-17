#pragma once

#include "post/TextUtils.h"

/// @file
/// @brief Каретка машины Поста.

namespace post {

/**
 * @brief Каретка (считывающая головка), указывающая на ячейку ленты.
 *
 * Сдвиг вправо выполняется оператором `++`, влево — оператором `--`.
 */
class Carriage {
public:
    /// Создаёт каретку над ячейкой @p position.
    explicit Carriage(Position position = 0);

    /// Координата ячейки под кареткой.
    Position position() const;

    /// Переставляет каретку над ячейкой @p position.
    void moveTo(Position position);

    /// Сдвиг на одну ячейку вправо (префиксная форма).
    Carriage& operator++();

    /// Сдвиг на одну ячейку вправо (постфиксная форма, возвращает прежнее положение).
    Carriage operator++(int);

    /// Сдвиг на одну ячейку влево (префиксная форма).
    Carriage& operator--();

    /// Сдвиг на одну ячейку влево (постфиксная форма, возвращает прежнее положение).
    Carriage operator--(int);

    /// Каретки равны, если стоят над одной ячейкой.
    bool operator==(const Carriage& other) const;

    /// Каретки стоят над разными ячейками.
    bool operator!=(const Carriage& other) const;

private:
    Position position_;
};

}  // namespace post
