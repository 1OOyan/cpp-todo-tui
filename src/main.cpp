#include <iostream>
#include <clocale>
#include <ncurses.h>
#include "app.hpp"

int main(int argc, char* argv[]) {
    // Set locale for proper character display
    setlocale(LC_ALL, "");
    
    // Initialize ncurses - CRITICAL: must be first
    WINDOW* main_win = initscr();
    if (!main_win) {
        std::cerr << "Failed to initialize ncurses" << std::endl;
        return 1;
    }
    
    // Set up terminal mode
    cbreak();           // Disable line buffering
    noecho();           // Don't echo input
    keypad(stdscr, TRUE); // Enable special keys
    curs_set(0);        // Hide cursor
    nodelay(stdscr, FALSE); // Blocking input (will change for main loop)
    
    // Enable colors
    if (has_colors()) {
        start_color();
        use_default_colors();
        
        // Color pairs
        init_pair(1, COLOR_WHITE, COLOR_BLUE);   // Header
        init_pair(2, COLOR_BLACK, COLOR_WHITE);  // Selected item
        init_pair(3, COLOR_GREEN, COLOR_BLACK);  // Completed
        init_pair(4, COLOR_YELLOW, COLOR_BLACK); // High priority
        init_pair(5, COLOR_CYAN, COLOR_BLACK);   // Medium priority
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);// Low priority
        init_pair(7, COLOR_RED, COLOR_BLACK);    // Delete confirm
        init_pair(8, COLOR_WHITE, COLOR_BLACK);  // Normal text
    }
    
    // Clear screen and set background
    erase();
    refresh();
    
    try {
        App app;
        app.run();
    } catch (const std::exception& e) {
        endwin();
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    // Clean up ncurses
    endwin();
    
    return 0;
}