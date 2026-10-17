#include <iostream>

#include "fifteen/FifteenMenu.h"

int main() {
    fifteen::FifteenMenu menu(std::cin, std::cout);
    menu.run();
    return 0;
}
