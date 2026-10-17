#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

#include "post/Program.h"

using post::CommandType;
using post::Program;
using post::Rule;

namespace {

Program makeProgram() {
    Program program;
    program.addRule(Rule(2, CommandType::Check, 3, 1));
    program.addRule(Rule(1, CommandType::MoveRight, 2));
    program.addRule(Rule(3, CommandType::Stop));
    return program;
}

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

}  // namespace

TEST(Program, NewProgramIsEmpty) {
    const Program program;
    EXPECT_TRUE(program.isEmpty());
    EXPECT_EQ(program.size(), 0u);
    EXPECT_THROW(static_cast<void>(program.firstRuleNumber()), std::logic_error);
}

TEST(Program, AddRuleStoresRulesSortedByNumber) {
    const Program program = makeProgram();
    EXPECT_EQ(program.size(), 3u);
    EXPECT_EQ(program.firstRuleNumber(), 1);
    int expectedNumber = 1;
    for (const auto& entry : program.rules()) {
        EXPECT_EQ(entry.first, expectedNumber);
        ++expectedNumber;
    }
}

TEST(Program, AddRuleRejectsDuplicateNumber) {
    Program program = makeProgram();
    EXPECT_THROW(program.addRule(Rule(2, CommandType::Stop)), std::invalid_argument);
    EXPECT_EQ(program.size(), 3u);
}

TEST(Program, RuleLookupFindsExistingRuleOnly) {
    const Program program = makeProgram();
    EXPECT_TRUE(program.hasRule(2));
    EXPECT_FALSE(program.hasRule(9));
    EXPECT_TRUE(program.rule(2) == Rule(2, CommandType::Check, 3, 1));
    EXPECT_THROW(static_cast<void>(program.rule(9)), std::out_of_range);
}

TEST(Program, RemoveRuleReportsWhetherRuleExisted) {
    Program program = makeProgram();
    EXPECT_TRUE(program.removeRule(2));
    EXPECT_FALSE(program.removeRule(2));
    EXPECT_EQ(program.size(), 2u);
}

TEST(Program, LoadSkipsBlankLinesAndComments) {
    std::istringstream input("# comment\n\n1 R 2\n   \n  2 !  \n# end\n");
    Program program;
    program.load(input);
    EXPECT_EQ(program.size(), 2u);
    EXPECT_TRUE(program.rule(2) == Rule(2, CommandType::Stop));
}

TEST(Program, LoadReportsLineNumberOfBadRule) {
    std::istringstream input("1 R 2\n2 Q 3\n");
    Program program;
    try {
        program.load(input);
        EXPECT_TRUE(false);
    } catch (const std::invalid_argument& error) {
        EXPECT_TRUE(contains(error.what(), "line 2"));
    }
}

TEST(Program, LoadRejectsDuplicateNumbers) {
    std::istringstream input("1 R 2\n1 L 2\n");
    Program program;
    EXPECT_THROW(program.load(input), std::invalid_argument);
}

TEST(Program, LoadReplacesRulesAndKeepsOldOnesOnFailure) {
    Program program = makeProgram();
    std::istringstream bad("1 R 2\n2 Q 3\n");
    EXPECT_THROW(program.load(bad), std::invalid_argument);
    EXPECT_EQ(program.size(), 3u);
    std::istringstream good("7 !\n");
    program.load(good);
    EXPECT_EQ(program.size(), 1u);
    EXPECT_TRUE(program.hasRule(7));
}

TEST(Program, EqualityComparesRules) {
    Program changed = makeProgram();
    changed.removeRule(3);
    EXPECT_TRUE(makeProgram() == makeProgram());
    EXPECT_FALSE(makeProgram() != makeProgram());
    EXPECT_TRUE(makeProgram() != changed);
}

TEST(Program, StreamRoundTripPreservesProgram) {
    const Program original = makeProgram();
    std::stringstream stream;
    stream << original;
    EXPECT_EQ(stream.str(), "1 R 2\n2 ? 3 1\n3 !\n");
    Program loaded;
    stream >> loaded;
    EXPECT_FALSE(stream.fail());
    EXPECT_TRUE(loaded == original);
}

TEST(Program, StreamInputFailsOnBadTextAndKeepsProgram) {
    Program program = makeProgram();
    std::istringstream input("1 Q 2\n");
    input >> program;
    EXPECT_TRUE(input.fail());
    EXPECT_TRUE(program == makeProgram());
}

TEST(Program, LoadAcceptsLastLineWithoutNewline) {
    std::istringstream input("1 R 2\n2 !");
    Program program;
    input >> program;
    EXPECT_FALSE(input.fail());
    EXPECT_EQ(program.size(), 2u);
}
