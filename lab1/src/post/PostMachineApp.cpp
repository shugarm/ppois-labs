#include "post/PostMachineApp.h"

#include <fstream>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "post/PostMachine.h"
#include "post/PostMachineMenu.h"

namespace post {

namespace {

constexpr const char* kLogFlag = "-log";
constexpr const char* kInteractiveFlag = "-i";
constexpr char kFlagPrefix = '-';
constexpr const char* kUsage = "Usage: post_machine <file> [-log] [-i]\n"
                               "  <file>  file with the tape, the carriage position and the program\n"
                               "  -log    print the machine state after every executed rule\n"
                               "  -i      open the interactive menu instead of running right away\n";
constexpr const char* kFinalStateTitle = "Final state:\n";

PostMachine loadMachine(const std::string& path) {
    PostMachine machine;
    if (path.empty()) {
        return machine;
    }
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    machine.load(file);
    return machine;
}

int runMachine(const Options& options, std::istream& input, std::ostream& output) {
    PostMachine machine = loadMachine(options.path);
    if (options.interactive) {
        PostMachineMenu menu(input, output, std::move(machine), options.log);
        menu.run();
        return kExitSuccess;
    }
    machine.run(PostMachine::kDefaultStepLimit, options.log ? &output : nullptr);
    output << kFinalStateTitle << machine;
    return kExitSuccess;
}

}  // namespace

Options parseArguments(const std::vector<std::string>& args) {
    Options options;
    for (const std::string& argument : args) {
        if (argument == kLogFlag) {
            options.log = true;
        } else if (argument == kInteractiveFlag) {
            options.interactive = true;
        } else if (!argument.empty() && argument.front() == kFlagPrefix) {
            throw std::invalid_argument("Unknown option: " + argument);
        } else if (!options.path.empty()) {
            throw std::invalid_argument("Only one file can be specified");
        } else {
            options.path = argument;
        }
    }
    if (options.path.empty() && !options.interactive) {
        throw std::invalid_argument("File path is required");
    }
    return options;
}

int runApplication(const std::vector<std::string>& args, std::istream& input, std::ostream& output,
                   std::ostream& errors) {
    Options options;
    try {
        options = parseArguments(args);
    } catch (const std::invalid_argument& error) {
        errors << "Error: " << error.what() << '\n' << kUsage;
        return kExitUsageError;
    }
    try {
        return runMachine(options, input, output);
    } catch (const std::exception& error) {
        errors << "Error: " << error.what() << '\n';
        return kExitFailure;
    }
}

}  // namespace post
