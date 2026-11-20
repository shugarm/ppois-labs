#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "post/PostMachineApp.h"

using post::Options;

namespace {

struct Result {
    int exitCode;
    std::string output;
    std::string errors;
};

bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

std::string writeTemporaryFile(const std::string& name, const std::string& content) {
    const std::string path = (std::filesystem::temp_directory_path() / name).string();
    std::ofstream file(path);
    file << content;
    return path;
}

Result run(const std::vector<std::string>& args, const std::string& commands = "") {
    std::istringstream input(commands);
    std::ostringstream output;
    std::ostringstream errors;
    const int exitCode = post::runApplication(args, input, output, errors);
    return Result{exitCode, output.str(), errors.str()};
}

const char* const kIncrementProgram = "tape: 11\ncarriage: 0\nprogram:\n1 R 2\n2 ? 1 3\n3 V 4\n4 !\n";

}  // namespace

TEST(PostMachineApp, ParseArgumentsReadsPathAndFlags) {
    const Options options = post::parseArguments({"machine.txt", "-log", "-i"});
    EXPECT_EQ(options.path, "machine.txt");
    EXPECT_TRUE(options.log);
    EXPECT_TRUE(options.interactive);
    const Options plain = post::parseArguments({"machine.txt"});
    EXPECT_FALSE(plain.log);
    EXPECT_FALSE(plain.interactive);
}

TEST(PostMachineApp, ParseArgumentsAllowsInteractiveModeWithoutFile) {
    const Options options = post::parseArguments({"-i"});
    EXPECT_TRUE(options.path.empty());
    EXPECT_TRUE(options.interactive);
}

TEST(PostMachineApp, ParseArgumentsRejectsBadCommandLines) {
    EXPECT_THROW(static_cast<void>(post::parseArguments({})), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(post::parseArguments({"-log"})), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(post::parseArguments({"a.txt", "b.txt"})), std::invalid_argument);
    EXPECT_THROW(static_cast<void>(post::parseArguments({"a.txt", "-x"})), std::invalid_argument);
}

TEST(PostMachineApp, UsageErrorPrintsHelp) {
    const Result result = run({});
    EXPECT_EQ(result.exitCode, post::kExitUsageError);
    EXPECT_TRUE(contains(result.errors, "Usage:"));
    EXPECT_TRUE(result.output.empty());
}

TEST(PostMachineApp, RunsProgramFromFileAndPrintsFinalState) {
    const std::string path = writeTemporaryFile("post_app_increment_test.txt", kIncrementProgram);
    const Result result = run({path});
    std::remove(path.c_str());
    EXPECT_EQ(result.exitCode, post::kExitSuccess);
    EXPECT_TRUE(contains(result.output, "Final state:"));
    EXPECT_TRUE(contains(result.output, "tape: 0:111"));
    EXPECT_FALSE(contains(result.output, "step 1 |"));
}

TEST(PostMachineApp, LogFlagPrintsEveryExecutedRule) {
    const std::string path = writeTemporaryFile("post_app_log_test.txt", kIncrementProgram);
    const Result result = run({path, "-log"});
    std::remove(path.c_str());
    EXPECT_EQ(result.exitCode, post::kExitSuccess);
    EXPECT_TRUE(contains(result.output, "step 1 | rule 1"));
    EXPECT_TRUE(contains(result.output, "step 6 | rule 4"));
}

TEST(PostMachineApp, MissingFileIsReported) {
    const Result result = run({"definitely_missing_post_machine_file.txt"});
    EXPECT_EQ(result.exitCode, post::kExitFailure);
    EXPECT_TRUE(contains(result.errors, "Cannot open file"));
}

TEST(PostMachineApp, BrokenFileIsReported) {
    const std::string path = writeTemporaryFile("post_app_broken_test.txt", "program:\n1 Q 2\n");
    const Result result = run({path});
    std::remove(path.c_str());
    EXPECT_EQ(result.exitCode, post::kExitFailure);
    EXPECT_TRUE(contains(result.errors, "Error:"));
}

TEST(PostMachineApp, RuntimeErrorIsReported) {
    const std::string path = writeTemporaryFile("post_app_runtime_error_test.txt", "tape: 1\nprogram:\n1 V 2\n2 !\n");
    const Result result = run({path});
    std::remove(path.c_str());
    EXPECT_EQ(result.exitCode, post::kExitFailure);
    EXPECT_TRUE(contains(result.errors, "already marked"));
}

TEST(PostMachineApp, InteractiveModeWorksWithoutFile) {
    const Result result = run({"-i"}, "0\n");
    EXPECT_EQ(result.exitCode, post::kExitSuccess);
    EXPECT_TRUE(contains(result.output, "Машина Поста"));
}

TEST(PostMachineApp, InteractiveModeLoadsFileFirst) {
    const std::string path = writeTemporaryFile("post_app_interactive_test.txt", kIncrementProgram);
    const Result result = run({path, "-i"}, "2\n0\n");
    std::remove(path.c_str());
    EXPECT_EQ(result.exitCode, post::kExitSuccess);
    EXPECT_TRUE(contains(result.output, "2 ? 1 3"));
}
