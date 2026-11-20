#include "post/Tape.h"

#include <cctype>
#include <istream>
#include <ostream>
#include <stdexcept>

namespace post {

Tape::Tape(const std::string& text) {
    const std::size_t separator = text.find(kOriginSeparator);
    if (separator == std::string::npos) {
        loadCells(text, 0);
        return;
    }
    const Position origin = parseInteger(text.substr(0, separator));
    loadCells(text.substr(separator + 1), origin);
}

void Tape::loadCells(const std::string& cells, Position origin) {
    Position position = origin;
    for (const char symbol : cells) {
        if (std::isspace(static_cast<unsigned char>(symbol))) {
            continue;
        }
        if (symbol == kMarkedChar) {
            markedCells_.insert(position);
        } else if (symbol != kEmptyChar) {
            throw std::invalid_argument(std::string("Unexpected tape symbol: '") + symbol + "'");
        }
        ++position;
    }
}

bool Tape::isMarked(Position position) const {
    return markedCells_.count(position) != 0;
}

bool Tape::operator[](Position position) const {
    return isMarked(position);
}

void Tape::mark(Position position) {
    markedCells_.insert(position);
}

void Tape::erase(Position position) {
    markedCells_.erase(position);
}

void Tape::set(Position position, bool marked) {
    if (marked) {
        mark(position);
    } else {
        erase(position);
    }
}

bool Tape::isEmpty() const {
    return markedCells_.empty();
}

std::size_t Tape::markedCount() const {
    return markedCells_.size();
}

Position Tape::leftmost() const {
    if (markedCells_.empty()) {
        throw std::logic_error("The tape has no marks");
    }
    return *markedCells_.begin();
}

Position Tape::rightmost() const {
    if (markedCells_.empty()) {
        throw std::logic_error("The tape has no marks");
    }
    return *markedCells_.rbegin();
}

std::string Tape::render(Position from, Position to) const {
    std::string cells;
    for (Position position = from; position <= to; ++position) {
        cells += isMarked(position) ? kMarkedChar : kEmptyChar;
    }
    return cells;
}

bool Tape::operator==(const Tape& other) const {
    return markedCells_ == other.markedCells_;
}

bool Tape::operator!=(const Tape& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& output, const Tape& tape) {
    if (tape.isEmpty()) {
        return output << Position{0} << kOriginSeparator;
    }
    return output << tape.leftmost() << kOriginSeparator << tape.render(tape.leftmost(), tape.rightmost());
}

std::istream& operator>>(std::istream& input, Tape& tape) {
    std::string line;
    if (!std::getline(input, line)) {
        return input;
    }
    try {
        tape = Tape(line);
    } catch (const std::invalid_argument&) {
        input.setstate(std::ios::failbit);
    }
    return input;
}

}  // namespace post
