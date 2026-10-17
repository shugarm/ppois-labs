#include "post/Program.h"

#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <string>

#include "post/TextUtils.h"

namespace post {

namespace {

void addParsedRule(std::map<int, Rule>& rules, const std::string& text, std::size_t lineNumber) {
    const std::string where = "line " + std::to_string(lineNumber) + ": ";
    try {
        const Rule rule = Rule::parse(text);
        if (!rules.emplace(rule.number(), rule).second) {
            throw std::invalid_argument("duplicate rule number " + std::to_string(rule.number()));
        }
    } catch (const std::invalid_argument& error) {
        throw std::invalid_argument(where + error.what());
    }
}

}  // namespace

void Program::addRule(const Rule& rule) {
    if (!rules_.emplace(rule.number(), rule).second) {
        throw std::invalid_argument("Rule " + std::to_string(rule.number()) + " already exists");
    }
}

bool Program::removeRule(int number) {
    return rules_.erase(number) != 0;
}

bool Program::hasRule(int number) const {
    return rules_.count(number) != 0;
}

const Rule& Program::rule(int number) const {
    const auto found = rules_.find(number);
    if (found == rules_.end()) {
        throw std::out_of_range("Rule " + std::to_string(number) + " does not exist");
    }
    return found->second;
}

const std::map<int, Rule>& Program::rules() const {
    return rules_;
}

std::size_t Program::size() const {
    return rules_.size();
}

bool Program::isEmpty() const {
    return rules_.empty();
}

int Program::firstRuleNumber() const {
    if (rules_.empty()) {
        throw std::logic_error("The program has no rules");
    }
    return rules_.begin()->first;
}

void Program::load(std::istream& input) {
    std::map<int, Rule> loaded;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        const std::string text = trim(line);
        if (!isIgnorableLine(text)) {
            addParsedRule(loaded, text, lineNumber);
        }
    }
    clearEndOfFileFailure(input);
    rules_ = std::move(loaded);
}

bool Program::operator==(const Program& other) const {
    return rules_ == other.rules_;
}

bool Program::operator!=(const Program& other) const {
    return !(*this == other);
}

std::ostream& operator<<(std::ostream& output, const Program& program) {
    for (const auto& entry : program.rules()) {
        output << entry.second << '\n';
    }
    return output;
}

std::istream& operator>>(std::istream& input, Program& program) {
    try {
        program.load(input);
    } catch (const std::invalid_argument&) {
        input.setstate(std::ios::failbit);
    }
    return input;
}

}  // namespace post
