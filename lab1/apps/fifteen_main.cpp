#include <iostream>

#include "console/Utf8Console.h"
#include "fifteen/FifteenMenu.h"

int main() {
    console::enableUtf8();
    fifteen::FifteenMenu menu(std::cin, std::cout);
    menu.run();
    return 0;
}
