#pragma once

#include <cstddef>
#include <iosfwd>
#include <vector>

/// @file
/// @brief Класс игры-головоломки «Пятнашки».

/// Классы игры «Пятнашки».
namespace fifteen {

/// Координаты клетки игрового поля (нумерация с нуля).
struct Position {
    std::size_t row;  ///< Номер строки.
    std::size_t col;  ///< Номер столбца.
};

/// Сравнение координат на равенство.
bool operator==(const Position& left, const Position& right);

/// Сравнение координат на неравенство.
bool operator!=(const Position& left, const Position& right);

/**
 * @brief Игровое поле «Пятнашки» размером size x size.
 *
 * Клетки хранятся в `std::vector`, поэтому деструктор, конструктор копирования и
 * оператор присваивания копированием, сгенерированные компилятором, работают корректно
 * (правило нуля): ручное управление памятью не требуется.
 *
 * Число 0 обозначает пустую клетку. Решённая головоломка выглядит так:
 * числа 1..size*size-1 по порядку, а пустая клетка находится в правом нижнем углу.
 */
class FifteenPuzzle {
public:
    /// Минимально допустимый размер поля.
    static constexpr std::size_t kMinSize = 2;
    /// Максимально допустимый размер поля.
    static constexpr std::size_t kMaxSize = 10;
    /// Размер поля по умолчанию (классические пятнашки).
    static constexpr std::size_t kDefaultSize = 4;
    /// Значение, обозначающее пустую клетку.
    static constexpr int kEmptyTile = 0;

    /// Представление строки поля, позволяющее писать `puzzle[row][col]`.
    class RowView {
    public:
        /**
         * @brief Значение клетки в данной строке.
         * @param col номер столбца
         * @throws std::out_of_range если столбец вне поля
         */
        int operator[](std::size_t col) const;

    private:
        friend class FifteenPuzzle;
        RowView(const FifteenPuzzle& puzzle, std::size_t row);

        const FifteenPuzzle& puzzle_;
        std::size_t row_;
    };

    /**
     * @brief Создаёт поле со случайным (но всегда разрешимым) расположением плиток.
     * @param size размер стороны поля
     * @throws std::invalid_argument если размер вне диапазона [kMinSize, kMaxSize]
     */
    explicit FifteenPuzzle(std::size_t size = kDefaultSize);

    /**
     * @brief Создаёт поле с заданным расположением плиток (по строкам).
     * @param size размер стороны поля
     * @param tiles size*size чисел: перестановка 0..size*size-1
     * @throws std::invalid_argument если размер или набор плиток некорректны
     */
    FifteenPuzzle(std::size_t size, const std::vector<int>& tiles);

    /// Размер стороны поля.
    std::size_t size() const;

    /**
     * @brief Доступ к строке поля: `puzzle[row][col]` возвращает значение клетки.
     * @param row номер строки
     * @throws std::out_of_range если строка вне поля
     */
    RowView operator[](std::size_t row) const;

    /// Координаты пустой клетки.
    Position emptyPosition() const;

    /// Координаты плиток, которые сейчас можно сдвинуть (соседи пустой клетки).
    std::vector<Position> movableTiles() const;

    /// Можно ли сдвинуть плитку, лежащую в указанной клетке.
    bool canMove(Position position) const;

    /**
     * @brief Переставляет плитку из указанной клетки на место пустой.
     * @return true, если перестановка выполнена; false, если плитку сдвинуть нельзя
     */
    bool moveTile(Position position);

    /**
     * @brief Переставляет плитку с указанным номером на место пустой.
     * @return true, если перестановка выполнена; false, если такой плитки нет
     *         или она не граничит с пустой клеткой
     */
    bool moveTileByValue(int value);

    /// Случайно перемешивает плитки (начальное значение генератора непредсказуемо).
    void shuffle();

    /// Перемешивает плитки детерминированно для данного значения @p seed (удобно для тестов).
    void shuffle(unsigned seed);

    /// Проверяет, что все плитки стоят на своих местах.
    bool isSolved() const;

    /// Проверяет, что текущую расстановку можно привести к решённой.
    bool isSolvable() const;

    /// Сравнение полей на равенство.
    bool operator==(const FifteenPuzzle& other) const;

    /// Сравнение полей на неравенство.
    bool operator!=(const FifteenPuzzle& other) const;

private:
    std::size_t indexOf(Position position) const;
    Position positionOf(std::size_t index) const;
    std::size_t emptyIndex() const;

    std::size_t size_;
    std::vector<int> tiles_;
};

/**
 * @brief Записывает поле в поток: в первой строке размер, затем строки поля (0 — пустая клетка).
 */
std::ostream& operator<<(std::ostream& output, const FifteenPuzzle& puzzle);

/**
 * @brief Читает поле из потока в формате, который выдаёт operator<<.
 *
 * При неверном формате или некорректной расстановке устанавливает failbit,
 * а @p puzzle остаётся без изменений.
 */
std::istream& operator>>(std::istream& input, FifteenPuzzle& puzzle);

}  // namespace fifteen
