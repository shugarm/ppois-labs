#include "post/PostMachineMenu.h"

#include <istream>
#include <ostream>
#include <sstream>
#include <utility>
#include <vector>

namespace post {

namespace {

constexpr const char* kMenuText =
    "\n=== Машина Поста ===\n"
    "1. Показать состояние машины\n"
    "2. Показать правила\n"
    "3. Добавить правило\n"
    "4. Удалить правило\n"
    "5. Изменить ячейку ленты\n"
    "6. Выполнить один шаг\n"
    "7. Выполнить программу до остановки\n"
    "8. Вернуться к началу программы\n"
    "0. Выход\n";

constexpr const char* kChoicePrompt = "Ваш выбор: ";
constexpr const char* kInvalidChoiceMessage = "Введите номер пункта меню.\n";
constexpr const char* kUnknownItemMessage = "Такого пункта в меню нет.\n";
constexpr const char* kRulePrompt = "Правило (например, '3 ? 1 2'): ";
constexpr const char* kRuleNumberPrompt = "Номер правила: ";
constexpr const char* kPositionPrompt = "Координата ячейки: ";
constexpr const char* kValuePrompt = "Значение (1 - метка, 0 - пусто): ";
constexpr const char* kInvalidNumberMessage = "Нужно ввести число.\n";
constexpr const char* kInvalidValueMessage = "Значение должно быть 0 или 1.\n";
constexpr const char* kRuleAddedMessage = "Правило добавлено.\n";
constexpr const char* kRuleRemovedMessage = "Правило удалено.\n";
constexpr const char* kNoSuchRuleMessage = "Правила с таким номером нет.\n";
constexpr const char* kNoRulesMessage = "Правил нет.\n";
constexpr const char* kCellChangedMessage = "Ячейка изменена.\n";
constexpr const char* kHaltedMessage = "Машина остановлена.\n";
constexpr const char* kProgramFinishedMessage = "Программа завершена, шагов: ";
constexpr const char* kResetMessage = "Машина возвращена к началу программы.\n";
constexpr const char* kErrorPrefix = "Ошибка: ";

constexpr long long kMarkedValue = 1;
constexpr long long kEmptyValue = 0;

}  // namespace

PostMachineMenu::PostMachineMenu(std::istream& input, std::ostream& output, PostMachine machine, bool logEnabled)
    : input_(input), output_(output), machine_(std::move(machine)), logEnabled_(logEnabled) {}

const PostMachine& PostMachineMenu::machine() const {
    return machine_;
}

void PostMachineMenu::run() {
    bool running = true;
    while (running && input_) {
        output_ << kMenuText;
        running = processChoice();
    }
}

bool PostMachineMenu::processChoice() {
    const std::optional<long long> choice = readNumber(kChoicePrompt);
    if (!input_) {
        return false;
    }
    if (!choice) {
        output_ << kInvalidChoiceMessage;
        return true;
    }
    return execute(static_cast<int>(*choice));
}

PostMachineMenu::Action PostMachineMenu::findAction(int choice) const {
    static const std::vector<std::pair<Item, Action>> actions = {
        {Item::ShowState, &PostMachineMenu::showState},   {Item::ListRules, &PostMachineMenu::listRules},
        {Item::AddRule, &PostMachineMenu::addRule},       {Item::RemoveRule, &PostMachineMenu::removeRule},
        {Item::SetCell, &PostMachineMenu::setCell},       {Item::Step, &PostMachineMenu::step},
        {Item::Run, &PostMachineMenu::runProgram},        {Item::Reset, &PostMachineMenu::resetMachine}};
    for (const auto& entry : actions) {
        if (static_cast<int>(entry.first) == choice) {
            return entry.second;
        }
    }
    return nullptr;
}

bool PostMachineMenu::execute(int choice) {
    if (choice == static_cast<int>(Item::Exit)) {
        return false;
    }
    const Action action = findAction(choice);
    if (action == nullptr) {
        output_ << kUnknownItemMessage;
        return true;
    }
    try {
        (this->*action)();
    } catch (const std::exception& error) {
        output_ << kErrorPrefix << error.what() << '\n';
    }
    return true;
}

void PostMachineMenu::showState() {
    output_ << machine_ << "state: " << machine_.describe() << '\n';
}

void PostMachineMenu::listRules() {
    if (machine_.program().isEmpty()) {
        output_ << kNoRulesMessage;
        return;
    }
    output_ << machine_.program();
}

void PostMachineMenu::addRule() {
    const std::string line = readLine(kRulePrompt);
    machine_.addRule(Rule::parse(line));
    output_ << kRuleAddedMessage;
}

void PostMachineMenu::removeRule() {
    const std::optional<long long> number = readNumber(kRuleNumberPrompt);
    if (!number) {
        output_ << kInvalidNumberMessage;
        return;
    }
    const bool removed = machine_.removeRule(static_cast<int>(*number));
    output_ << (removed ? kRuleRemovedMessage : kNoSuchRuleMessage);
}

void PostMachineMenu::setCell() {
    const std::optional<long long> position = readNumber(kPositionPrompt);
    const std::optional<long long> value = readNumber(kValuePrompt);
    if (!position || !value) {
        output_ << kInvalidNumberMessage;
        return;
    }
    if (*value != kMarkedValue && *value != kEmptyValue) {
        output_ << kInvalidValueMessage;
        return;
    }
    machine_.setCell(*position, *value == kMarkedValue);
    output_ << kCellChangedMessage;
}

void PostMachineMenu::step() {
    if (!machine_.step()) {
        output_ << kHaltedMessage;
        return;
    }
    output_ << "rule executed; " << machine_.describe() << '\n';
}

void PostMachineMenu::runProgram() {
    std::ostream* log = logEnabled_ ? &output_ : nullptr;
    const std::size_t steps = machine_.run(PostMachine::kDefaultStepLimit, log);
    output_ << kProgramFinishedMessage << steps << '\n' << machine_.describe() << '\n';
}

void PostMachineMenu::resetMachine() {
    machine_.reset();
    output_ << kResetMessage;
}

std::optional<long long> PostMachineMenu::readNumber(const std::string& prompt) {
    const std::string line = readLine(prompt);
    std::istringstream parser(line);
    long long value = 0;
    char trailing = 0;
    if (!input_ || !(parser >> value) || (parser >> trailing)) {
        return std::nullopt;
    }
    return value;
}

std::string PostMachineMenu::readLine(const std::string& prompt) {
    output_ << prompt;
    std::string line;
    std::getline(input_, line);
    return line;
}

}  // namespace post
