#include "app.hpp"
#include <ncurses.h>
#include <algorithm>
#include <sstream>
#include <clocale>
#include <cstring>
#include <unistd.h>

App::App() : m_db("todo.db") {
    refreshTodos();
    getmaxyx(stdscr, m_screenHeight, m_screenWidth);
}

App::~App() = default;

void App::refreshTodos() {
    if (m_currentFilter == todo::TodoStatus::Active) {
        m_todos = m_db.getTodosByStatus(todo::TodoStatus::Active);
    } else if (m_currentFilter == todo::TodoStatus::Completed) {
        m_todos = m_db.getTodosByStatus(todo::TodoStatus::Completed);
    } else {
        m_todos = m_db.getAllTodos();
    }
    
    if (m_selectedIndex >= static_cast<int>(m_todos.size())) {
        m_selectedIndex = m_todos.empty() ? 0 : static_cast<int>(m_todos.size()) - 1;
    }
    if (m_selectedIndex < 0) m_selectedIndex = 0;
    
    int listHeight = m_screenHeight - 6;
    if (m_scrollOffset > m_selectedIndex) {
        m_scrollOffset = m_selectedIndex;
    }
    if (m_scrollOffset + listHeight <= m_selectedIndex && listHeight > 0) {
        m_scrollOffset = m_selectedIndex - listHeight + 1;
    }
}

void App::run() {
    // Main loop with non-blocking input
    nodelay(stdscr, TRUE);
    
    while (true) {
        showMainScreen();
        int ch = getch();
        
        switch (ch) {
            case 'q':
            case KEY_F(10):
            case 27: // ESC
                return;
            case 'n':
                showAddDialog();
                break;
            case 'e':
                if (!m_todos.empty()) showEditDialog(m_todos[m_selectedIndex]);
                break;
            case 'd':
                if (!m_todos.empty()) showDeleteConfirm(m_todos[m_selectedIndex]);
                break;
            case 't':
                if (!m_todos.empty()) toggleTodo(m_selectedIndex);
                break;
            case 'f':
                filterTodos();
                break;
            case 's':
                searchTodos();
                break;
            case 'h':
            case '?':
                showHelpDialog();
                break;
            case KEY_UP:
                if (m_selectedIndex > 0) m_selectedIndex--;
                break;
            case KEY_DOWN:
                if (m_selectedIndex < static_cast<int>(m_todos.size()) - 1) m_selectedIndex++;
                break;
            case KEY_PPAGE: // Page Up
                m_scrollOffset = std::max(0, m_scrollOffset - (m_screenHeight - 6));
                break;
            case KEY_NPAGE: // Page Down
                m_scrollOffset = std::min(static_cast<int>(m_todos.size()) - (m_screenHeight - 6), 
                                         m_scrollOffset + (m_screenHeight - 6));
                break;
            case KEY_RESIZE:
                getmaxyx(stdscr, m_screenHeight, m_screenWidth);
                break;
        }
        
        // Small delay to prevent CPU spinning
        napms(30);
    }
}

void App::showMainScreen() {
    // Clear the screen
    erase();
    
    // Draw all UI elements
    renderHeader();
    showTodoList(m_todos);
    renderStatusLine();
    renderFooter();
    
    // Refresh the screen
    refresh();
}

void App::renderHeader() {
    // Draw header box
    int y = 0;
    attron(A_BOLD | COLOR_PAIR(1));
    
    // Top border
    mvhline(y++, 0, ACS_HLINE, m_screenWidth);
    
    // Title
    mvprintw(y++, 2, " ╔════════════════════════════════════════════════════════════════════════════╗ ");
    mvprintw(y++, 2, " ║                    ★  C++ TODO LIST  ★                                     ║ ");
    mvprintw(y++, 2, " ╚════════════════════════════════════════════════════════════════════════════╝ ");
    
    // Bottom border
    mvhline(y++, 0, ACS_HLINE, m_screenWidth);
    
    attroff(A_BOLD | COLOR_PAIR(1));
}

void App::renderFooter() {
    int footerY = m_screenHeight - 2;
    
    // Draw footer box
    attron(A_BOLD | COLOR_PAIR(1));
    mvhline(footerY, 0, ACS_HLINE, m_screenWidth);
    
    // Help text
    mvprintw(footerY + 1, 2, " [n]New  [e]Edit  [d]Delete  [t]Toggle  [f]Filter  [s]Search  [h]Help  [q]Quit ");
    
    attroff(A_BOLD | COLOR_PAIR(1));
}

void App::renderStatusLine() {
    int statusY = m_screenHeight - 3;
    std::stringstream ss;
    
    int total = m_db.getTotalCount();
    int active = m_db.getActiveCount();
    int completed = m_db.getCompletedCount();
    
    ss << " 📊 Stats: Total=" << total << " | Active=" << active << " | Done=" << completed;
    
    // Filter info
    switch (m_currentFilter) {
        case todo::TodoStatus::Active:
            ss << " | 🔍 Filter: Active";
            break;
        case todo::TodoStatus::Completed:
            ss << " | ✅ Filter: Completed";
            break;
        default:
            ss << " | 🔍 Filter: All";
            break;
    }
    
    if (!m_searchQuery.empty()) {
        ss << " | 🔎 Search: \"" << m_searchQuery << "\"";
    }
    
    attron(A_REVERSE);
    mvprintw(statusY, 2, "%-*s", m_screenWidth - 4, ss.str().c_str());
    attroff(A_REVERSE);
}

void App::showTodoList(const std::vector<todo::Todo>& todos) {
    int listStartY = 3;
    int listHeight = m_screenHeight - 6;
    
    // Draw list header
    attron(A_BOLD);
    mvprintw(listStartY, 2, " ┌───────────────────────────────────────────────────────────────────────────────┐");
    mvprintw(listStartY + 1, 3, " │ Status │ Priority │ Task Description                                        │");
    mvprintw(listStartY + 2, 2, " ├────────┼──────────┼─────────────────────────────────────────────────────────┤");
    attroff(A_BOLD);
    
    if (todos.empty()) {
        // Empty state
        int emptyY = listStartY + listHeight / 2;
        attron(A_BOLD | COLOR_PAIR(6));
        mvprintw(emptyY, (m_screenWidth - 40) / 2, "📝 No todos yet! Press [n] to add your first todo.");
        attroff(A_BOLD | COLOR_PAIR(6));
        
        // Draw bottom border
        mvprintw(listStartY + listHeight - 1, 2, " └───────────────────────────────────────────────────────────────────────────────┘");
        return;
    }
    
    int visibleCount = std::min(listHeight - 3, static_cast<int>(todos.size()) - m_scrollOffset);
    
    for (int i = 0; i < visibleCount; i++) {
        int actualIndex = m_scrollOffset + i;
        if (actualIndex >= static_cast<int>(todos.size())) break;
        
        const auto& todo = todos[actualIndex];
        int y = listStartY + 3 + i;
        
        // Check if this is the selected item
        bool isSelected = (actualIndex == m_selectedIndex);
        
        // Draw row background if selected
        if (isSelected) {
            attron(COLOR_PAIR(2));
            mvhline(y, 2, ' ', m_screenWidth - 4);
        }
        
        // Status column
        std::string status = todo.isCompleted() ? " [x] " : " [ ] ";
        if (todo.isCompleted()) {
            attron(COLOR_PAIR(3));
        }
        mvprintw(y, 4, "%s", status.c_str());
        if (todo.isCompleted()) attroff(COLOR_PAIR(3));
        
        // Priority column
        std::string prio, prioColor;
        switch (todo.priority) {
            case todo::Priority::High:
                prio = " HIGH ";
                attron(COLOR_PAIR(4));
                break;
            case todo::Priority::Medium:
                prio = " MEDIUM ";
                attron(COLOR_PAIR(5));
                break;
            case todo::Priority::Low:
                prio = "  LOW  ";
                attron(COLOR_PAIR(6));
                break;
        }
        mvprintw(y, 11, "%s", prio.c_str());
        attroff(COLOR_PAIR(4) | COLOR_PAIR(5) | COLOR_PAIR(6));
        
        // Description column (truncated)
        std::string desc = todo.description;
        int maxDescLen = m_screenWidth - 35;
        if (desc.length() > static_cast<size_t>(maxDescLen)) {
            desc = desc.substr(0, maxDescLen - 3) + "...";
        }
        mvprintw(y, 22, " %-*s", maxDescLen, desc.c_str());
        
        // Reset attributes
        if (isSelected) attroff(COLOR_PAIR(2));
    }
    
    // Draw bottom border
    mvprintw(listStartY + visibleCount + 2, 2, " └───────────────────────────────────────────────────────────────────────────────┘");
    
    // Scroll indicator if needed
    if (m_scrollOffset > 0) {
        mvprintw(listStartY + 1, m_screenWidth - 8, " ▲ ");
    }
    if (m_scrollOffset + visibleCount < static_cast<int>(todos.size())) {
        mvprintw(listStartY + visibleCount + 1, m_screenWidth - 8, " ▼ ");
    }
}

void App::showAddDialog() {
    // Switch to blocking input for dialog
    nodelay(stdscr, FALSE);
    echo();
    curs_set(1);
    
    int dialogY = std::max(0, m_screenHeight / 2 - 5);
    int dialogX = std::max(0, (m_screenWidth - 60) / 2);
    
    // Clear dialog area
    for (int i = 0; i < 8; i++) {
        mvhline(dialogY + i, dialogX, ' ', 60);
    }
    
    // Draw dialog box
    attron(A_BOLD | COLOR_PAIR(1));
    mvhline(dialogY, dialogX, ACS_HLINE, 60);
    mvhline(dialogY + 7, dialogX, ACS_HLINE, 60);
    for (int i = 0; i < 8; i++) {
        mvvline(dialogY + i, dialogX, ACS_VLINE, 1);
        mvvline(dialogY + i, dialogX + 59, ACS_VLINE, 1);
    }
    mvhline(dialogY, dialogX, ACS_ULCORNER, 1);
    mvhline(dialogY, dialogX + 59, ACS_URCORNER, 1);
    mvhline(dialogY + 7, dialogX, ACS_LLCORNER, 1);
    mvhline(dialogY + 7, dialogX + 59, ACS_LRCORNER, 1);
    mvprintw(dialogY, dialogX + 23, " ADD NEW TODO ");
    attroff(A_BOLD | COLOR_PAIR(1));
    
    mvprintw(dialogY + 2, dialogX + 3, "Description: ");
    mvprintw(dialogY + 4, dialogX + 3, "Priority (1=Low, 2=Medium, 3=High): ");
    mvprintw(dialogY + 5, dialogX + 3, "Press [Enter] to save, [Esc] to cancel");
    
    refresh();
    
    // Get description
    char desc[256] = {0};
    move(dialogY + 2, dialogX + 17);
    getnstr(desc, 255);
    
    if (desc[0] == 27 || strlen(desc) == 0) {
        curs_set(0);
        noecho();
        nodelay(stdscr, TRUE);
        return;
    }
    
    // Get priority
    char prioStr[10] = {0};
    move(dialogY + 4, dialogX + 45);
    getnstr(prioStr, 9);
    
    int prio = strlen(prioStr) > 0 ? std::max(1, std::min(3, atoi(prioStr))) : 1;
    
    todo::Todo newTodo;
    newTodo.description = desc;
    newTodo.priority = static_cast<todo::Priority>(prio);
    m_db.createTodo(newTodo);
    refreshTodos();
    
    curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);
}

void App::showEditDialog(const todo::Todo& todo) {
    nodelay(stdscr, FALSE);
    echo();
    curs_set(1);
    
    int dialogY = std::max(0, m_screenHeight / 2 - 5);
    int dialogX = std::max(0, (m_screenWidth - 60) / 2);
    
    // Clear dialog area
    for (int i = 0; i < 8; i++) {
        mvhline(dialogY + i, dialogX, ' ', 60);
    }
    
    attron(A_BOLD | COLOR_PAIR(1));
    mvhline(dialogY, dialogX, ACS_HLINE, 60);
    mvhline(dialogY + 7, dialogX, ACS_HLINE, 60);
    for (int i = 0; i < 8; i++) {
        mvvline(dialogY + i, dialogX, ACS_VLINE, 1);
        mvvline(dialogY + i, dialogX + 59, ACS_VLINE, 1);
    }
    mvhline(dialogY, dialogX, ACS_ULCORNER, 1);
    mvhline(dialogY, dialogX + 59, ACS_URCORNER, 1);
    mvhline(dialogY + 7, dialogX, ACS_LLCORNER, 1);
    mvhline(dialogY + 7, dialogX + 59, ACS_LRCORNER, 1);
    mvprintw(dialogY, dialogX + 23, " EDIT TODO ");
    attroff(A_BOLD | COLOR_PAIR(1));
    
    mvprintw(dialogY + 2, dialogX + 3, "Description: ");
    mvprintw(dialogY + 3, dialogX + 17, "%s", todo.description.c_str());
    mvprintw(dialogY + 4, dialogX + 3, "Priority (1=Low, 2=Medium, 3=High): ");
    mvprintw(dialogY + 4, dialogX + 45, "%d", static_cast<int>(todo.priority));
    mvprintw(dialogY + 5, dialogX + 3, "Press [Enter] to save, [Esc] to cancel");
    
    refresh();
    
    char desc[256] = {0};
    move(dialogY + 2, dialogX + 17);
    getnstr(desc, 255);
    
    if (desc[0] == 27) {
        curs_set(0);
        noecho();
        nodelay(stdscr, TRUE);
        return;
    }
    
    std::string newDesc = strlen(desc) > 0 ? desc : todo.description;
    char prioStr[10] = {0};
    move(dialogY + 4, dialogX + 45);
    getnstr(prioStr, 9);
    
    int prio = strlen(prioStr) > 0 ? std::max(1, std::min(3, atoi(prioStr))) : static_cast<int>(todo.priority);
    
    todo::Todo updated = todo;
    updated.description = newDesc;
    updated.priority = static_cast<todo::Priority>(prio);
    m_db.updateTodo(updated);
    refreshTodos();
    
    curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);
}

void App::showDeleteConfirm(const todo::Todo& todo) {
    nodelay(stdscr, FALSE);
    echo();
    curs_set(1);
    
    int dialogY = std::max(0, m_screenHeight / 2 - 4);
    int dialogX = std::max(0, (m_screenWidth - 50) / 2);
    
    // Clear dialog area
    for (int i = 0; i < 7; i++) {
        mvhline(dialogY + i, dialogX, ' ', 50);
    }
    
    attron(A_BOLD | COLOR_PAIR(7));
    mvhline(dialogY, dialogX, ACS_HLINE, 50);
    mvhline(dialogY + 6, dialogX, ACS_HLINE, 50);
    for (int i = 0; i < 7; i++) {
        mvvline(dialogY + i, dialogX, ACS_VLINE, 1);
        mvvline(dialogY + i, dialogX + 49, ACS_VLINE, 1);
    }
    mvhline(dialogY, dialogX, ACS_ULCORNER, 1);
    mvhline(dialogY, dialogX + 49, ACS_URCORNER, 1);
    mvhline(dialogY + 6, dialogX, ACS_LLCORNER, 1);
    mvhline(dialogY + 6, dialogX + 49, ACS_LRCORNER, 1);
    mvprintw(dialogY, dialogX + 17, " DELETE TODO ");
    attroff(A_BOLD | COLOR_PAIR(7));
    
    mvprintw(dialogY + 2, dialogX + 3, "Delete this todo?");
    mvprintw(dialogY + 3, dialogX + 3, "\"%s\"", todo.description.c_str());
    mvprintw(dialogY + 4, dialogX + 3, "Press [Y] to confirm, any other key to cancel");
    
    refresh();
    
    int ch = getch();
    if (ch == 'y' || ch == 'Y') {
        m_db.deleteTodo(todo.id);
        if (m_selectedIndex >= static_cast<int>(m_todos.size()) - 1) {
            m_selectedIndex = m_todos.empty() ? 0 : static_cast<int>(m_todos.size()) - 1;
        }
        refreshTodos();
    }
    
    curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);
}

void App::showHelpDialog() {
    nodelay(stdscr, FALSE);
    echo();
    curs_set(1);
    
    int dialogY = std::max(0, m_screenHeight / 2 - 8);
    int dialogX = std::max(0, (m_screenWidth - 60) / 2);
    
    // Clear dialog area
    for (int i = 0; i < 16; i++) {
        mvhline(dialogY + i, dialogX, ' ', 60);
    }
    
    attron(A_BOLD | COLOR_PAIR(1));
    mvhline(dialogY, dialogX, ACS_HLINE, 60);
    mvhline(dialogY + 15, dialogX, ACS_HLINE, 60);
    for (int i = 0; i < 16; i++) {
        mvvline(dialogY + i, dialogX, ACS_VLINE, 1);
        mvvline(dialogY + i, dialogX + 59, ACS_VLINE, 1);
    }
    mvhline(dialogY, dialogX, ACS_ULCORNER, 1);
    mvhline(dialogY, dialogX + 59, ACS_URCORNER, 1);
    mvhline(dialogY + 15, dialogX, ACS_LLCORNER, 1);
    mvhline(dialogY + 15, dialogX + 59, ACS_LRCORNER, 1);
    mvprintw(dialogY, dialogX + 25, " HELP - KEYBOARD SHORTCUTS ");
    attroff(A_BOLD | COLOR_PAIR(1));
    
    const char* helpText[] = {
        "  n     - Add new todo",
        "  e     - Edit selected todo",
        "  d     - Delete selected todo",
        "  t     - Toggle completion status",
        "  f     - Filter (All/Active/Completed)",
        "  s     - Search todos",
        "  ↑/↓   - Navigate list",
        "  PgUp/Dn - Page through list",
        "  h/?   - Show this help",
        "  q     - Quit application",
        "",
        "  Tip: High priority items show in yellow"
    };
    
    for (int i = 0; i < 12; i++) {
        mvprintw(dialogY + 2 + i, dialogX + 3, "%s", helpText[i]);
    }
    
    mvprintw(dialogY + 14, dialogX + 3, "Press any key to continue...");
    
    refresh();
    getch();
    
    curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);
}

void App::showSearchDialog() {
    nodelay(stdscr, FALSE);
    echo();
    curs_set(1);
    
    int dialogY = std::max(0, m_screenHeight / 2 - 3);
    int dialogX = std::max(0, (m_screenWidth - 40) / 2);
    
    // Clear dialog area
    for (int i = 0; i < 5; i++) {
        mvhline(dialogY + i, dialogX, ' ', 40);
    }
    
    attron(A_BOLD | COLOR_PAIR(1));
    mvhline(dialogY, dialogX, ACS_HLINE, 40);
    mvhline(dialogY + 4, dialogX, ACS_HLINE, 40);
    for (int i = 0; i < 5; i++) {
        mvvline(dialogY + i, dialogX, ACS_VLINE, 1);
        mvvline(dialogY + i, dialogX + 39, ACS_VLINE, 1);
    }
    mvhline(dialogY, dialogX, ACS_ULCORNER, 1);
    mvhline(dialogY, dialogX + 39, ACS_URCORNER, 1);
    mvhline(dialogY + 4, dialogX, ACS_LLCORNER, 1);
    mvhline(dialogY + 4, dialogX + 39, ACS_LRCORNER, 1);
    mvprintw(dialogY, dialogX + 12, " SEARCH TODOS ");
    attroff(A_BOLD | COLOR_PAIR(1));
    
    mvprintw(dialogY + 1, dialogX + 3, "Enter search text:");
    mvprintw(dialogY + 3, dialogX + 3, "Press [Enter] to search, [Esc] to cancel");
    
    refresh();
    
    char query[128] = {0};
    move(dialogY + 1, dialogX + 22);
    getnstr(query, 127);
    
    if (query[0] != 27) {
        m_searchQuery = query;
        if (!m_searchQuery.empty()) {
            std::vector<todo::Todo> results = m_db.searchTodos(m_searchQuery);
            
            // Show results
            std::vector<todo::Todo> oldTodos = m_todos;
            m_todos = results;
            m_selectedIndex = 0;
            m_scrollOffset = 0;
            
            showMainScreen();
            
            attron(A_BOLD | COLOR_PAIR(4));
            mvprintw(m_screenHeight - 3, 2, " Found %zu results. Press any key to return...", results.size());
            attroff(A_BOLD | COLOR_PAIR(4));
            
            refresh();
            getch();
            
            m_todos = oldTodos;
            m_searchQuery.clear();
            refreshTodos();
        }
    } else {
        m_searchQuery.clear();
    }
    
    curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);
}

void App::filterTodos() {
    switch (m_currentFilter) {
        case todo::TodoStatus::Active:
            m_currentFilter = todo::TodoStatus::Completed;
            break;
        case todo::TodoStatus::Completed:
            m_currentFilter = todo::TodoStatus::Active;
            break;
        default:
            m_currentFilter = todo::TodoStatus::Active;
            break;
    }
    refreshTodos();
}

void App::searchTodos() {
    showSearchDialog();
}

void App::toggleTodo(int index) {
    if (index >= 0 && index < static_cast<int>(m_todos.size())) {
        todo::Todo todo = m_todos[index];
        todo.status = todo.isCompleted() ? todo::TodoStatus::Active : todo::TodoStatus::Completed;
        m_db.updateTodo(todo);
        refreshTodos();
    }
}

void App::deleteTodo(int index) {
    if (index >= 0 && index < static_cast<int>(m_todos.size())) {
        todo::Todo todo = m_todos[index];
        m_db.deleteTodo(todo.id);
        if (m_selectedIndex >= static_cast<int>(m_todos.size()) - 1) {
            m_selectedIndex = m_todos.empty() ? 0 : static_cast<int>(m_todos.size()) - 1;
        }
        refreshTodos();
    }
}

void App::selectTodo(int index) {
    if (index >= 0 && index < static_cast<int>(m_todos.size())) {
        m_selectedIndex = index;
    }
}