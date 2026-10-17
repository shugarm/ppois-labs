#include "post/PostMachine.h"

#include <algorithm>
#include <istream>
#include <ostream>
#include <sstream>
#include <utility>

namespace post {

namespace {

constexpr char kHeaderSeparator = ':';
constexpr const char* kTapeKey = "tape";
constexpr const char* kCarriageKey = "carriage";
constexpr const char* kProgramKey = "program";
constexpr char kCarriageOpen = '[';
constexpr char kCarriageClose = ']';

struct Header {
    std::string key;
    std::string value;
};

Header splitHeader(const std::string& text) {
    const std::size_t separator = text.find(kHeaderSeparator);
    if (separator == std::string::npos) {
        throw std::invalid_argument("Expected a 'key: value' line, got: '" + text + "'");
    }
    return Header{trim(text.substr(0, separator)), trim(text.substr(separator + 1))};
}

bool applyHeader(const Header& header, std::istream& input, Tape& tape, Carriage& carriage, Program& program) {
    if (header.key == kProgramKey) {
        program.load(input);
        return true;
    }
    if (header.key == kTapeKey) {
        tape = Tape(header.value);
    } else if (header.key == kCarriageKey) {
        carriage = Carriage(parseInteger(header.value));
    } else {
        throw std::invalid_argument("Unknown section: '" + header.key + "'");
    }
    return false;
}

}  // namespace

PostMachine::PostMachine(Tape tape, Carriage carriage, Program program)
    : tape_(std::move(tape)), carriage_(carriage), program_(std::move(program)) {
    reset();
}

void PostMachine::loadProgram(std::istream& input) {
    Program loaded;
    loaded.load(input);
    program_ = std::move(loaded);
    reset();
}

void PostMachine::loadTape(std::istream& input) {
    Tape loaded;
    if (!(input >> loaded)) {
        throw std::invalid_argument("Invalid tape description");
    }
    tape_ = std::move(loaded);
}

void PostMachine::load(std::istream& input) {
    Tape tape;
    Carriage carriage;
    Program program;
    std::string line;
    bool programRead = false;
    while (!programRead && std::getline(input, line)) {
        const std::string text = trim(line);
        if (!isIgnorableLine(text)) {
            programRead = applyHeader(splitHeader(text), input, tape, carriage, program);
        }
    }
    clearEndOfFileFailure(input);
    *this = PostMachine(std::move(tape), carriage, std::move(program));
}

void PostMachine::addRule(const Rule& rule) {
    program_.addRule(rule);
    if (currentRuleNumber_ == Rule::kNoTarget) {
        currentRuleNumber_ = program_.firstRuleNumber();
    }
}

bool PostMachine::removeRule(int number) {
    return program_.removeRule(number);
}

const Program& PostMachine::program() const {
    return program_;
}

const Tape& PostMachine::tape() const {
    return tape_;
}

const Carriage& PostMachine::carriage() const {
    return carriage_;
}

bool PostMachine::cell(Position position) const {
    return tape_.isMarked(position);
}

void PostMachine::setCell(Position position, bool marked) {
    tape_.set(position, marked);
}

void PostMachine::moveCarriageTo(Position position) {
    carriage_.moveTo(position);
}

int PostMachine::currentRuleNumber() const {
    return currentRuleNumber_;
}

bool PostMachine::isHalted() const {
    return halted_;
}

void PostMachine::reset() {
    halted_ = false;
    currentRuleNumber_ = program_.isEmpty() ? Rule::kNoTarget : program_.firstRuleNumber();
}

const Rule& PostMachine::currentRule() const {
    if (program_.isEmpty()) {
        throw PostMachineError("The program has no rules");
    }
    if (!program_.hasRule(currentRuleNumber_)) {
        throw PostMachineError("Rule " + std::to_string(currentRuleNumber_) + " is not defined");
    }
    return program_.rule(currentRuleNumber_);
}

void PostMachine::markCurrentCell() {
    if (tape_.isMarked(carriage_.position())) {
        throw PostMachineError("Cannot mark a cell that is already marked: " + std::to_string(carriage_.position()));
    }
    tape_.mark(carriage_.position());
}

void PostMachine::eraseCurrentCell() {
    if (!tape_.isMarked(carriage_.position())) {
        throw PostMachineError("Cannot erase a cell that is already empty: " + std::to_string(carriage_.position()));
    }
    tape_.erase(carriage_.position());
}

void PostMachine::applyCommand(const Rule& rule) {
    switch (rule.type()) {
        case CommandType::Mark:
            markCurrentCell();
            break;
        case CommandType::Erase:
            eraseCurrentCell();
            break;
        case CommandType::MoveLeft:
            --carriage_;
            break;
        case CommandType::MoveRight:
            ++carriage_;
            break;
        case CommandType::Check:
        case CommandType::Stop:
            break;
    }
}

int PostMachine::nextRuleNumber(const Rule& rule) const {
    if (rule.type() == CommandType::Check) {
        return tape_.isMarked(carriage_.position()) ? rule.firstTarget() : rule.secondTarget();
    }
    return rule.firstTarget();
}

bool PostMachine::step() {
    if (halted_) {
        return false;
    }
    const Rule rule = currentRule();
    const int next = nextRuleNumber(rule);
    applyCommand(rule);
    if (rule.type() == CommandType::Stop) {
        halted_ = true;
    } else {
        currentRuleNumber_ = next;
    }
    return true;
}

std::size_t PostMachine::run(std::size_t stepLimit, std::ostream* log) {
    std::size_t executed = 0;
    while (!halted_) {
        if (executed == stepLimit) {
            throw PostMachineError("Step limit exceeded: " + std::to_string(stepLimit));
        }
        const int ruleNumber = currentRuleNumber_;
        step();
        ++executed;
        if (log != nullptr) {
            *log << "step " << executed << " | rule " << ruleNumber << " | " << describe() << '\n';
        }
    }
    return executed;
}

std::string PostMachine::renderTapeWindow() const {
    const Position carriagePosition = carriage_.position();
    Position from = carriagePosition;
    Position to = carriagePosition;
    if (!tape_.isEmpty()) {
        from = std::min(from, tape_.leftmost());
        to = std::max(to, tape_.rightmost());
    }
    std::string window;
    for (Position position = from; position <= to; ++position) {
        const char symbol = tape_.isMarked(position) ? kMarkedChar : kEmptyChar;
        if (position == carriagePosition) {
            window += std::string{kCarriageOpen, symbol, kCarriageClose};
        } else {
            window += symbol;
        }
    }
    return window;
}

std::string PostMachine::describe() const {
    std::ostringstream text;
    text << "carriage=" << carriage_.position() << " tape=" << renderTapeWindow();
    return text.str();
}

bool PostMachine::operator==(const PostMachine& other) const {
    return tape_ == other.tape_ && carriage_ == other.carriage_ && program_ == other.program_ &&
           currentRuleNumber_ == other.currentRuleNumber_ && halted_ == other.halted_;
}

bool PostMachine::operator!=(const PostMachine& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& output, const PostMachine& machine) {
    output << kTapeKey << kHeaderSeparator << ' ' << machine.tape() << '\n';
    output << kCarriageKey << kHeaderSeparator << ' ' << machine.carriage().position() << '\n';
    output << kProgramKey << kHeaderSeparator << '\n' << machine.program();
    return output;
}

std::istream& operator>>(std::istream& input, PostMachine& machine) {
    try {
        machine.load(input);
    } catch (const std::invalid_argument&) {
        input.setstate(std::ios::failbit);
    }
    return input;
}

}  // namespace post
