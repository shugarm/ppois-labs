#include "post/Carriage.h"

namespace post {

Carriage::Carriage(Position position) : position_(position) {}

Position Carriage::position() const {
    return position_;
}

void Carriage::moveTo(Position position) {
    position_ = position;
}

Carriage& Carriage::operator++() {
    ++position_;
    return *this;
}

Carriage Carriage::operator++(int) {
    const Carriage previous = *this;
    ++position_;
    return previous;
}

Carriage& Carriage::operator--() {
    --position_;
    return *this;
}

Carriage Carriage::operator--(int) {
    const Carriage previous = *this;
    --position_;
    return previous;
}

bool Carriage::operator==(const Carriage& other) const {
    return position_ == other.position_;
}

bool Carriage::operator!=(const Carriage& other) const {
    return !(*this == other);
}

}  // namespace post
