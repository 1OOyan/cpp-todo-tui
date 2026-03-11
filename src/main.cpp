#include <iostream>
#include <clocale>
#include "app.hpp"

int main() {
    std::setlocale(LC_ALL, "");
    try {
        App app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}