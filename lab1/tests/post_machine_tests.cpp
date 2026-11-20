#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "post/PostMachine.h"

using post::Carriage;
using post::CommandType;
using post::PostMachine;
using post::PostMachineError;
using post::Program;
using post::Rule;
using post::Tape;

namespace {

constexpr std::size_t kSmallStepLimit = 50;

const char* const kAddOneFile =
    "# unary increment\n"
    "tape: 111\n"
    "carriage: 0\n"
    "program:\n"
    "1 R 2\n"
    "2 ? 1 3\n"
    "3 V 4\n"
    "4 !\n";

PostMachine loadMachine(const std::string& text) {
    std::istringstream input(text);
    PostMachine machine;
    machine.load(input);
    return machine;
}

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

}  // namespace

TEST(PostMachine, NewMachineIsEmpty) {
    const PostMachine machine;
    EXPECT_TRUE(machine.tape().isEmpty());
    EXPECT_EQ(machine.carriage().position(), 0);
    EXPECT_TRUE(machine.program().isEmpty());
    EXPECT_EQ(machine.currentRuleNumber(), Rule::kNoTarget);
    EXPECT_FALSE(machine.isHalted());
}

TEST(PostMachine, ConstructorFromPartsStartsAtFirstRule) {
    Program program;
    program.addRule(Rule(5, CommandType::Stop));
    const PostMachine machine(Tape("1"), Carriage(2), program);
    EXPECT_EQ(machine.currentRuleNumber(), 5);
    EXPECT_EQ(machine.carriage().position(), 2);
    EXPECT_TRUE(machine.cell(0));
}

TEST(PostMachine, LoadReadsTapeCarriageAndProgram) {
    const PostMachine machine = loadMachine(kAddOneFile);
    EXPECT_TRUE(machine.tape() == Tape("111"));
    EXPECT_EQ(machine.carriage().position(), 0);
    EXPECT_EQ(machine.program().size(), 4u);
    EXPECT_EQ(machine.currentRuleNumber(), 1);
}

TEST(PostMachine, LoadAcceptsNegativeCarriageAndTapeOrigin) {
    const PostMachine machine = loadMachine("tape: -2:101\ncarriage: -1\nprogram:\n1 !\n");
    EXPECT_EQ(machine.carriage().position(), -1);
    EXPECT_TRUE(machine.cell(-2));
    EXPECT_FALSE(machine.cell(-1));
}

TEST(PostMachine, LoadAllowsMissingSections) {
    const PostMachine onlyTape = loadMachine("tape: 11\n");
    EXPECT_TRUE(onlyTape.program().isEmpty());
    EXPECT_EQ(onlyTape.tape().markedCount(), 2u);
    const PostMachine empty = loadMachine("");
    EXPECT_TRUE(empty == PostMachine());
}

TEST(PostMachine, LoadRejectsBrokenFilesAndKeepsMachine) {
    const std::vector<std::string> badFiles = {"tape 111\n",       "color: red\n",       "tape: 12\n",
                                               "carriage: left\n", "program:\n1 Q 2\n", "program:\n1 R 2\n1 R 3\n"};
    for (const std::string& text : badFiles) {
        PostMachine machine = loadMachine(kAddOneFile);
        std::istringstream input(text);
        EXPECT_THROW(machine.load(input), std::invalid_argument);
        EXPECT_TRUE(machine == loadMachine(kAddOneFile));
    }
}

TEST(PostMachine, LoadProgramReplacesProgramOnly) {
    PostMachine machine = loadMachine(kAddOneFile);
    std::istringstream input("9 !\n");
    machine.loadProgram(input);
    EXPECT_EQ(machine.program().size(), 1u);
    EXPECT_EQ(machine.currentRuleNumber(), 9);
    EXPECT_TRUE(machine.tape() == Tape("111"));
}

TEST(PostMachine, LoadTapeReplacesTapeOnly) {
    PostMachine machine = loadMachine(kAddOneFile);
    std::istringstream input("1:1\n");
    machine.loadTape(input);
    EXPECT_TRUE(machine.tape() == Tape("1:1"));
    EXPECT_EQ(machine.program().size(), 4u);
    std::istringstream bad("abc\n");
    EXPECT_THROW(machine.loadTape(bad), std::invalid_argument);
}

TEST(PostMachine, AddAndRemoveRulesEditProgram) {
    PostMachine machine;
    machine.addRule(Rule(3, CommandType::Stop));
    EXPECT_EQ(machine.currentRuleNumber(), 3);
    machine.addRule(Rule(1, CommandType::Mark, 3));
    EXPECT_EQ(machine.currentRuleNumber(), 3);
    EXPECT_THROW(machine.addRule(Rule(1, CommandType::Stop)), std::invalid_argument);
    EXPECT_TRUE(machine.removeRule(1));
    EXPECT_FALSE(machine.removeRule(1));
    EXPECT_EQ(machine.program().size(), 1u);
}

TEST(PostMachine, CellsAndCarriageCanBeEdited) {
    PostMachine machine;
    machine.setCell(4, true);
    EXPECT_TRUE(machine.cell(4));
    machine.setCell(4, false);
    EXPECT_FALSE(machine.cell(4));
    machine.moveCarriageTo(-3);
    EXPECT_EQ(machine.carriage().position(), -3);
}

TEST(PostMachine, StepExecutesOneRuleAtATime) {
    PostMachine machine = loadMachine(kAddOneFile);
    EXPECT_TRUE(machine.step());
    EXPECT_EQ(machine.carriage().position(), 1);
    EXPECT_EQ(machine.currentRuleNumber(), 2);
    EXPECT_TRUE(machine.step());
    EXPECT_EQ(machine.currentRuleNumber(), 1);
}

TEST(PostMachine, RunIncrementsUnaryNumber) {
    PostMachine machine = loadMachine(kAddOneFile);
    const std::size_t steps = machine.run();
    EXPECT_EQ(steps, 8u);
    EXPECT_TRUE(machine.isHalted());
    EXPECT_TRUE(machine.tape() == Tape("1111"));
    EXPECT_EQ(machine.carriage().position(), 3);
}

TEST(PostMachine, StepAfterStopDoesNothing) {
    PostMachine machine = loadMachine(kAddOneFile);
    machine.run();
    EXPECT_FALSE(machine.step());
    EXPECT_EQ(machine.run(), 0u);
}

TEST(PostMachine, CheckBranchesOnCellValue) {
    PostMachine marked = loadMachine("tape: 1\nprogram:\n1 ? 2 3\n2 !\n3 !\n");
    marked.step();
    EXPECT_EQ(marked.currentRuleNumber(), 2);
    PostMachine empty = loadMachine("tape: 0\nprogram:\n1 ? 2 3\n2 !\n3 !\n");
    empty.step();
    EXPECT_EQ(empty.currentRuleNumber(), 3);
}

TEST(PostMachine, MarkEraseAndMoveLeftWork) {
    PostMachine machine = loadMachine("carriage: 0\nprogram:\n1 L 2\n2 V 3\n3 R 4\n4 V 5\n5 X 6\n6 !\n");
    machine.run();
    EXPECT_TRUE(machine.tape() == Tape("-1:1"));
    EXPECT_EQ(machine.carriage().position(), 0);
}

TEST(PostMachine, MarkingMarkedCellIsAnError) {
    PostMachine machine = loadMachine("tape: 1\nprogram:\n1 V 2\n2 !\n");
    EXPECT_THROW(machine.step(), PostMachineError);
}

TEST(PostMachine, ErasingEmptyCellIsAnError) {
    PostMachine machine = loadMachine("program:\n1 X 2\n2 !\n");
    EXPECT_THROW(machine.step(), PostMachineError);
}

TEST(PostMachine, EmptyProgramCannotRun) {
    PostMachine machine;
    EXPECT_THROW(machine.step(), PostMachineError);
}

TEST(PostMachine, JumpToMissingRuleIsAnError) {
    PostMachine machine = loadMachine("program:\n1 R 7\n");
    EXPECT_TRUE(machine.step());
    EXPECT_THROW(machine.step(), PostMachineError);
}

TEST(PostMachine, RunStopsAtStepLimit) {
    PostMachine machine = loadMachine("program:\n1 R 1\n");
    EXPECT_THROW(machine.run(kSmallStepLimit), PostMachineError);
}

TEST(PostMachine, RunWritesLogLinePerRule) {
    PostMachine machine = loadMachine("tape: 1\nprogram:\n1 R 2\n2 !\n");
    std::ostringstream log;
    machine.run(PostMachine::kDefaultStepLimit, &log);
    EXPECT_EQ(log.str(), "step 1 | rule 1 | carriage=1 tape=1[0]\nstep 2 | rule 2 | carriage=1 tape=1[0]\n");
}

TEST(PostMachine, ResetReturnsToFirstRuleKeepingTape) {
    PostMachine machine = loadMachine(kAddOneFile);
    machine.run();
    machine.reset();
    EXPECT_FALSE(machine.isHalted());
    EXPECT_EQ(machine.currentRuleNumber(), 1);
    EXPECT_TRUE(machine.tape() == Tape("1111"));
}

TEST(PostMachine, DescribeShowsCarriageInsideAndOutsideTape) {
    PostMachine machine = loadMachine("tape: 101\ncarriage: 1\nprogram:\n1 !\n");
    EXPECT_EQ(machine.describe(), "carriage=1 tape=1[0]1");
    machine.moveCarriageTo(3);
    EXPECT_EQ(machine.describe(), "carriage=3 tape=101[0]");
    machine.moveCarriageTo(-2);
    EXPECT_EQ(machine.describe(), "carriage=-2 tape=[0]0101");
    EXPECT_EQ(PostMachine().describe(), "carriage=0 tape=[0]");
}

TEST(PostMachine, OutputUsesFileFormat) {
    std::ostringstream output;
    output << loadMachine(kAddOneFile);
    EXPECT_EQ(output.str(), "tape: 0:111\ncarriage: 0\nprogram:\n1 R 2\n2 ? 1 3\n3 V 4\n4 !\n");
}

TEST(PostMachine, StreamRoundTripPreservesMachine) {
    const PostMachine original = loadMachine(kAddOneFile);
    std::stringstream stream;
    stream << original;
    PostMachine loaded;
    stream >> loaded;
    EXPECT_FALSE(stream.fail());
    EXPECT_TRUE(loaded == original);
}

TEST(PostMachine, StreamInputFailsOnBrokenFile) {
    PostMachine machine = loadMachine(kAddOneFile);
    std::istringstream input("nonsense\n");
    input >> machine;
    EXPECT_TRUE(input.fail());
    EXPECT_TRUE(machine == loadMachine(kAddOneFile));
}

TEST(PostMachine, EqualityComparesTapeCarriageProgramAndState) {
    const PostMachine base = loadMachine(kAddOneFile);
    PostMachine moved = base;
    moved.moveCarriageTo(5);
    PostMachine edited = base;
    edited.setCell(10, true);
    PostMachine stepped = base;
    stepped.step();
    PostMachine otherProgram = base;
    otherProgram.removeRule(4);
    EXPECT_TRUE(base == loadMachine(kAddOneFile));
    EXPECT_FALSE(base != loadMachine(kAddOneFile));
    EXPECT_TRUE(base != moved);
    EXPECT_TRUE(base != edited);
    EXPECT_TRUE(base != stepped);
    EXPECT_TRUE(base != otherProgram);
}

TEST(PostMachine, ErrorMessagesAreInformative) {
    PostMachine machine = loadMachine("tape: 1\nprogram:\n1 V 2\n");
    try {
        machine.step();
        EXPECT_TRUE(false);
    } catch (const PostMachineError& error) {
        EXPECT_TRUE(contains(error.what(), "already marked"));
    }
}
