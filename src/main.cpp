#include <iostream>
#include <clocale>
#include <ncurses.h>
#include "app.hpp"

int main() {
    // Set locale for proper character display
    std::setlocale(LC_ALL, "");
    
    // Initialize ncurses FIRST
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, FALSE);  // Changed to FALSE for blocking input
    curs_set(0);  // Hide cursor
    
    // Enable colors
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Header
        init_pair(2, COLOR_BLACK, COLOR_WHITE);  // Selected item
        init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Completed
        init_pair(4, COLOR_YELLOW, COLOR_BLACK); // High priority
        init_pair(5, COLOR_CYAN, COLOR_BLACK);   // Medium priority
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);// Low priority
        init_pair(7, COLOR_RED, COLOR_BLACK);    // Delete confirm
    }
    
    try {
        App app;
        app.run();
    } catch (const std::exception& e) {
        endwin();
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}