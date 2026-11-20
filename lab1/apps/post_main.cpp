#include <iostream>
#include <string>
#include <vector>

#include "console/Utf8Console.h"
#include "post/PostMachineApp.h"

int main(int argc, char* argv[]) {
    console::enableUtf8();
    const std::vector<std::string> args(argv + 1, argv + argc);
    return post::runApplication(args, std::cin, std::cout, std::cerr);
}
