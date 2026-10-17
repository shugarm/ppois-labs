#include "post/Rule.h"

#include <array>
#include <istream>
#include <ostream>
#include <sstream>
#include <stdexcept>

namespace post {

namespace {

constexpr char kMarkSymbol = 'V';
constexpr char kEraseSymbol = 'X';
constexpr char kLeftSymbol = 'L';
constexpr char kRightSymbol = 'R';
constexpr char kCheckSymbol = '?';
constexpr char kStopSymbol = '!';
constexpr std::size_t kNoTargets = 0;
constexpr std::size_t kSingleTarget = 1;
constexpr std::size_t kMaxTargets = 2;

using Targets = std::array<int, kMaxTargets>;

void validateTarget(int target) {
    if (target < Rule::kMinRuleNumber) {
        throw std::invalid_argument("Rule target must be a valid rule number");
    }
}

bool readTargets(std::istream& input, std::size_t count, Targets& targets) {
    for (std::size_t index = 0; index < count; ++index) {
        if (!(input >> targets[index])) {
            return false;
        }
    }
    return true;
}

}  // namespace

char commandSymbol(CommandType type) {
    switch (type) {
        case CommandType::Mark:
            return kMarkSymbol;
        case CommandType::Erase:
            return kEraseSymbol;
        case CommandType::MoveLeft:
            return kLeftSymbol;
        case CommandType::MoveRight:
            return kRightSymbol;
        case CommandType::Check:
            return kCheckSymbol;
        case CommandType::Stop:
            return kStopSymbol;
    }
    throw std::invalid_argument("Unknown command type");
}

CommandType commandTypeFromSymbol(char symbol) {
    switch (symbol) {
        case kMarkSymbol:
            return CommandType::Mark;
        case kEraseSymbol:
            return CommandType::Erase;
        case kLeftSymbol:
            return CommandType::MoveLeft;
        case kRightSymbol:
            return CommandType::MoveRight;
        case kCheckSymbol:
            return CommandType::Check;
        case kStopSymbol:
            return CommandType::Stop;
        default:
            throw std::invalid_argument(std::string("Unknown command symbol: '") + symbol + "'");
    }
}

std::size_t targetCount(CommandType type) {
    switch (type) {
        case CommandType::Check:
            return kMaxTargets;
        case CommandType::Stop:
            return kNoTargets;
        default:
            return kSingleTarget;
    }
}

Rule::Rule() : Rule(kMinRuleNumber, CommandType::Stop) {}

Rule::Rule(int number, CommandType type, int firstTarget, int secondTarget)
    : number_(number), type_(type), firstTarget_(kNoTarget), secondTarget_(kNoTarget) {
    if (number < kMinRuleNumber) {
        throw std::invalid_argument("Rule number must be positive");
    }
    const std::size_t needed = targetCount(type);
    if (needed >= kSingleTarget) {
        validateTarget(firstTarget);
        firstTarget_ = firstTarget;
    }
    if (needed >= kMaxTargets) {
        validateTarget(secondTarget);
        secondTarget_ = secondTarget;
    }
}

Rule Rule::parse(const std::string& line) {
    std::istringstream stream(line);
    Rule rule;
    if (!(stream >> rule)) {
        throw std::invalid_argument("Invalid rule: '" + line + "'");
    }
    std::string trailing;
    if (stream >> trailing) {
        throw std::invalid_argument("Unexpected text after the rule: '" + trailing + "'");
    }
    return rule;
}

int Rule::number() const {
    return number_;
}

CommandType Rule::type() const {
    return type_;
}

int Rule::firstTarget() const {
    return firstTarget_;
}

int Rule::secondTarget() const {
    return secondTarget_;
}

bool Rule::operator==(const Rule& other) const {
    return number_ == other.number_ && type_ == other.type_ && firstTarget_ == other.firstTarget_ &&
           secondTarget_ == other.secondTarget_;
}

bool Rule::operator!=(const Rule& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& output, const Rule& rule) {
    output << rule.number() << ' ' << commandSymbol(rule.type());
    if (rule.firstTarget() != Rule::kNoTarget) {
        output << ' ' << rule.firstTarget();
    }
    if (rule.secondTarget() != Rule::kNoTarget) {
        output << ' ' << rule.secondTarget();
    }
    return output;
}

std::istream& operator>>(std::istream& input, Rule& rule) {
    int number = 0;
    char symbol = 0;
    if (!(input >> number >> symbol)) {
        return input;
    }
    try {
        const CommandType type = commandTypeFromSymbol(symbol);
        Targets targets{Rule::kNoTarget, Rule::kNoTarget};
        if (!readTargets(input, targetCount(type), targets)) {
            return input;
        }
        rule = Rule(number, type, targets[0], targets[1]);
    } catch (const std::invalid_argument&) {
        input.setstate(std::ios::failbit);
    }
    return input;
}

}  // namespace post
