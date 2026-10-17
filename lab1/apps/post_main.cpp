#include <iostream>
#include <string>
#include <vector>

#include "post/PostMachineApp.h"

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    return post::runApplication(args, std::cin, std::cout, std::cerr);
}
