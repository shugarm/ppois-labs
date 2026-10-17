#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "post/Rule.h"

using post::CommandType;
using post::Rule;

namespace {

const std::vector<CommandType> kAllCommands = {CommandType::Mark,     CommandType::Erase, CommandType::MoveLeft,
                                                CommandType::MoveRight, CommandType::Check, CommandType::Stop};
constexpr int kInvalidEnumValue = 99;

}  // namespace

TEST(Rule, DefaultRuleIsStop) {
    const Rule rule;
    EXPECT_EQ(rule.number(), 1);
    EXPECT_TRUE(rule.type() == CommandType::Stop);
}

TEST(Rule, StoresNumberCommandAndTargets) {
    const Rule rule(4, CommandType::Check, 5, 6);
    EXPECT_EQ(rule.number(), 4);
    EXPECT_TRUE(rule.type() == CommandType::Check);
    EXPECT_EQ(rule.firstTarget(), 5);
    EXPECT_EQ(rule.secondTarget(), 6);
}

TEST(Rule, UnusedTargetsAreDropped) {
    const Rule stop(1, CommandType::Stop, 5, 6);
    EXPECT_EQ(stop.firstTarget(), Rule::kNoTarget);
    EXPECT_EQ(stop.secondTarget(), Rule::kNoTarget);
    const Rule mark(1, CommandType::Mark, 2, 3);
    EXPECT_EQ(mark.firstTarget(), 2);
    EXPECT_EQ(mark.secondTarget(), Rule::kNoTarget);
}

TEST(Rule, ConstructorRejectsInvalidNumbersAndTargets) {
    EXPECT_THROW(Rule(0, CommandType::Stop), std::invalid_argument);
    EXPECT_THROW(Rule(1, CommandType::Mark), std::invalid_argument);
    EXPECT_THROW(Rule(1, CommandType::Mark, 0), std::invalid_argument);
    EXPECT_THROW(Rule(1, CommandType::Check, 2), std::invalid_argument);
    EXPECT_THROW(Rule(1, CommandType::Check, 2, -5), std::invalid_argument);
}

TEST(Rule, CommandSymbolsRoundTrip) {
    for (const CommandType type : kAllCommands) {
        EXPECT_TRUE(post::commandTypeFromSymbol(post::commandSymbol(type)) == type);
    }
}

TEST(Rule, UnknownSymbolsAndCommandsAreRejected) {
    EXPECT_THROW(static_cast<void>(post::commandTypeFromSymbol('Z')), std::invalid_argument);
    const CommandType invalidType = static_cast<CommandType>(kInvalidEnumValue);
    EXPECT_THROW(static_cast<void>(post::commandSymbol(invalidType)), std::invalid_argument);
}

TEST(Rule, TargetCountDependsOnCommand) {
    EXPECT_EQ(post::targetCount(CommandType::Stop), 0u);
    EXPECT_EQ(post::targetCount(CommandType::Mark), 1u);
    EXPECT_EQ(post::targetCount(CommandType::MoveRight), 1u);
    EXPECT_EQ(post::targetCount(CommandType::Check), 2u);
}

TEST(Rule, ParseReadsEveryCommandForm) {
    EXPECT_TRUE(Rule::parse("1 V 2") == Rule(1, CommandType::Mark, 2));
    EXPECT_TRUE(Rule::parse("2 X 3") == Rule(2, CommandType::Erase, 3));
    EXPECT_TRUE(Rule::parse("3 L 4") == Rule(3, CommandType::MoveLeft, 4));
    EXPECT_TRUE(Rule::parse("  4 R 5  ") == Rule(4, CommandType::MoveRight, 5));
    EXPECT_TRUE(Rule::parse("5 ? 6 7") == Rule(5, CommandType::Check, 6, 7));
    EXPECT_TRUE(Rule::parse("6 !") == Rule(6, CommandType::Stop));
}

TEST(Rule, ParseRejectsMalformedLines) {
    const std::vector<std::string> badLines = {"",      "a V 2", "1 Z 2",   "1 V",  "1 ? 2",
                                               "0 V 1", "1 V 0", "1 V 2 3", "1 ! 2"};
    for (const std::string& line : badLines) {
        EXPECT_THROW(static_cast<void>(Rule::parse(line)), std::invalid_argument);
    }
}

TEST(Rule, OutputUsesTextFormat) {
    std::ostringstream output;
    output << Rule(5, CommandType::Check, 6, 7) << '|' << Rule(1, CommandType::Mark, 2) << '|'
           << Rule(3, CommandType::Stop);
    EXPECT_EQ(output.str(), "5 ? 6 7|1 V 2|3 !");
}

TEST(Rule, StreamRoundTripPreservesRule) {
    const Rule original(8, CommandType::Check, 2, 9);
    std::stringstream stream;
    stream << original;
    Rule loaded;
    stream >> loaded;
    EXPECT_FALSE(stream.fail());
    EXPECT_TRUE(loaded == original);
}

TEST(Rule, InputFailsAndKeepsRuleOnBadText) {
    Rule rule(2, CommandType::Mark, 3);
    std::istringstream badSymbol("1 Q 2");
    badSymbol >> rule;
    EXPECT_TRUE(badSymbol.fail());
    std::istringstream badTarget("1 V 0");
    badTarget >> rule;
    EXPECT_TRUE(badTarget.fail());
    std::istringstream missingTarget("1 ? 2");
    missingTarget >> rule;
    EXPECT_TRUE(missingTarget.fail());
    std::istringstream nothing("");
    nothing >> rule;
    EXPECT_TRUE(nothing.fail());
    EXPECT_TRUE(rule == Rule(2, CommandType::Mark, 3));
}

TEST(Rule, EqualityComparesAllFields) {
    EXPECT_TRUE(Rule(1, CommandType::Mark, 2) == Rule(1, CommandType::Mark, 2));
    EXPECT_TRUE(Rule(1, CommandType::Mark, 2) != Rule(2, CommandType::Mark, 2));
    EXPECT_TRUE(Rule(1, CommandType::Mark, 2) != Rule(1, CommandType::Erase, 2));
    EXPECT_TRUE(Rule(1, CommandType::Mark, 2) != Rule(1, CommandType::Mark, 3));
    EXPECT_TRUE(Rule(1, CommandType::Check, 2, 3) != Rule(1, CommandType::Check, 2, 4));
}
