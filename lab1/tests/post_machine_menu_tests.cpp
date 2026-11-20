#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <utility>

#include "post/PostMachineMenu.h"

using post::PostMachine;
using post::PostMachineMenu;
using post::Rule;

namespace {

const char* const kProgramText = "tape: 11\ncarriage: 0\nprogram:\n1 R 2\n2 ? 1 3\n3 V 4\n4 !\n";

PostMachine makeMachine() {
    std::istringstream input(kProgramText);
    PostMachine machine;
    machine.load(input);
    return machine;
}

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

struct Session {
    std::string output;
    PostMachine machine;
};

Session runMenu(const std::string& commands, PostMachine machine = makeMachine(), bool log = false) {
    std::istringstream input(commands);
    std::ostringstream output;
    PostMachineMenu menu(input, output, std::move(machine), log);
    menu.run();
    return Session{output.str(), menu.machine()};
}

}  // namespace

TEST(PostMachineMenu, ExitStopsTheLoop) {
    EXPECT_TRUE(contains(runMenu("0\n").output, "Машина Поста"));
}

TEST(PostMachineMenu, EndOfInputStopsTheLoop) {
    EXPECT_NO_THROW(runMenu(""));
    EXPECT_NO_THROW(runMenu("1"));
}

TEST(PostMachineMenu, InvalidChoicesAreReported) {
    const Session session = runMenu("abc\n42\n0\n");
    EXPECT_TRUE(contains(session.output, "Введите номер пункта меню"));
    EXPECT_TRUE(contains(session.output, "Такого пункта в меню нет"));
}

TEST(PostMachineMenu, ShowStatePrintsMachineAndDescription) {
    const Session session = runMenu("1\n0\n");
    EXPECT_TRUE(contains(session.output, "program:"));
    EXPECT_TRUE(contains(session.output, "state: carriage=0 tape=[1]1"));
}

TEST(PostMachineMenu, ListRulesShowsRulesOrAMessage) {
    EXPECT_TRUE(contains(runMenu("2\n0\n").output, "2 ? 1 3"));
    EXPECT_TRUE(contains(runMenu("2\n0\n", PostMachine()).output, "Правил нет"));
}

TEST(PostMachineMenu, AddRuleAddsParsedRule) {
    const Session session = runMenu("3\n9 !\n0\n");
    EXPECT_TRUE(session.machine.program().hasRule(9));
    EXPECT_TRUE(contains(session.output, "Правило добавлено"));
}

TEST(PostMachineMenu, AddRuleReportsBadInputWithoutStopping) {
    const Session session = runMenu("3\nnonsense\n3\n1 !\n0\n");
    EXPECT_EQ(session.machine.program().size(), 4u);
    EXPECT_TRUE(contains(session.output, "Ошибка:"));
}

TEST(PostMachineMenu, RemoveRuleHandlesAllCases) {
    const Session removed = runMenu("4\n3\n0\n");
    EXPECT_FALSE(removed.machine.program().hasRule(3));
    EXPECT_TRUE(contains(removed.output, "Правило удалено"));
    EXPECT_TRUE(contains(runMenu("4\n77\n0\n").output, "Правила с таким номером нет"));
    EXPECT_TRUE(contains(runMenu("4\nabc\n0\n").output, "Нужно ввести число"));
}

TEST(PostMachineMenu, SetCellChangesTape) {
    const Session session = runMenu("5\n7\n1\n5\n0\n0\n0\n");
    EXPECT_TRUE(session.machine.cell(7));
    EXPECT_FALSE(session.machine.cell(0));
    EXPECT_TRUE(contains(session.output, "Ячейка изменена"));
}

TEST(PostMachineMenu, SetCellRejectsBadValues) {
    EXPECT_TRUE(contains(runMenu("5\nabc\n1\n0\n").output, "Нужно ввести число"));
    EXPECT_TRUE(contains(runMenu("5\n1\nabc\n0\n").output, "Нужно ввести число"));
    EXPECT_TRUE(contains(runMenu("5\n1\n2\n0\n").output, "должно быть 0 или 1"));
}

TEST(PostMachineMenu, StepExecutesOneRuleAndReportsHalt) {
    const Session session = runMenu("6\n0\n");
    EXPECT_EQ(session.machine.carriage().position(), 1);
    EXPECT_TRUE(contains(session.output, "rule executed"));
    const Session halted = runMenu("7\n6\n0\n");
    EXPECT_TRUE(contains(halted.output, "Машина остановлена"));
}

TEST(PostMachineMenu, StepErrorIsReported) {
    const Session session = runMenu("6\n0\n", PostMachine());
    EXPECT_TRUE(contains(session.output, "Ошибка:"));
}

TEST(PostMachineMenu, RunExecutesWholeProgram) {
    const Session session = runMenu("7\n0\n");
    EXPECT_TRUE(session.machine.isHalted());
    EXPECT_TRUE(contains(session.output, "Программа завершена, шагов: 6"));
    EXPECT_FALSE(contains(session.output, "step 1 |"));
}

TEST(PostMachineMenu, RunWithLogPrintsEveryRule) {
    const Session session = runMenu("7\n0\n", makeMachine(), true);
    EXPECT_TRUE(contains(session.output, "step 1 | rule 1"));
    EXPECT_TRUE(contains(session.output, "step 6 | rule 4"));
}

TEST(PostMachineMenu, ResetReturnsToStart) {
    const Session session = runMenu("7\n8\n0\n");
    EXPECT_FALSE(session.machine.isHalted());
    EXPECT_EQ(session.machine.currentRuleNumber(), 1);
    EXPECT_TRUE(contains(session.output, "возвращена к началу"));
}
