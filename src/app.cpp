#include "app.hpp"
#include <ncurses.h>
#include <algorithm>
#include <sstream>
#include <clocale>
#include <cstring>

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
        m_selectedIndex = m_todos.empty() ? 0 : m_todos.size() - 1;
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
    // ncurses is already initialized in main()
    while (true) {
        showMainScreen();
        int ch = getch();
        
        switch (ch) {
            case 'q':
            case KEY_F(10):
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
            case KEY_RESIZE:
                getmaxyx(stdscr, m_screenHeight, m_screenWidth);
                break;
        }
    }
}

void App::showMainScreen() {
    erase();
    renderHeader();
    showTodoList(m_todos);
    renderFooter();
    renderStatusLine();
    refresh();
}

void App::renderHeader() {
    attron(COLOR_PAIR(1));
    mvhline(0, 0, ACS_HLINE, m_screenWidth);
    mvprintw(1, 2, "  TODO LIST  ");
    attroff(COLOR_PAIR(1));
    mvhline(2, 0, ACS_HLINE, m_screenWidth);
}

void App::renderFooter() {
    int footerY = m_screenHeight - 3;
    mvhline(footerY, 0, ACS_HLINE, m_screenWidth);
    attron(COLOR_PAIR(1));
    mvprintw(footerY + 1, 2, " n:New  e:Edit  d:Delete  t:Toggle  f:Filter  s:Search  h:Help  q:Quit ");
    attroff(COLOR_PAIR(1));
}

void App::renderStatusLine() {
    int statusY = m_screenHeight - 2;
    std::stringstream ss;
    int total = m_db.getTotalCount();
    int active = m_db.getActiveCount();
    int completed = m_db.getCompletedCount();
    ss << " Total: " << total << " | Active: " << active << " | Completed: " << completed;
    switch (m_currentFilter) {
        case todo::TodoStatus::Active: ss << " | Filter: Active"; break;
        case todo::TodoStatus::Completed: ss << " | Filter: Completed"; break;
        default: ss << " | Filter: All"; break;
    }
    if (!m_searchQuery.empty()) ss << " | Search: " << m_searchQuery;
    mvprintw(statusY, 2, "%s", ss.str().c_str());
}

void App::showTodoList(const std::vector<todo::Todo>& todos) {
    int listStartY = 3;
    int listHeight = m_screenHeight - 6;
    if (todos.empty()) {
        mvprintw(listStartY + listHeight / 2, (m_screenWidth - 40) / 2, "No todos yet. Press 'n' to add one!");
        return;
    }
    int visibleCount = std::min(listHeight, static_cast<int>(todos.size()));
    for (int i = 0; i < visibleCount; i++) {
        int actualIndex = m_scrollOffset + i;
        if (actualIndex >= static_cast<int>(todos.size())) break;
        const auto& todo = todos[actualIndex];
        int y = listStartY + i;
        std::string line = todo.getStatusString() + " ";
        switch (todo.priority) {
            case todo::Priority::High:
                line += "[!!!] ";
                if (has_colors()) attron(COLOR_PAIR(4));
                break;
            case todo::Priority::Medium:
                line += "[--] ";
                if (has_colors()) attron(COLOR_PAIR(5));
                break;
            case todo::Priority::Low:
                line += "[...] ";
                if (has_colors()) attron(COLOR_PAIR(6));
                break;
        }
        line += todo.description;
        if (line.length() > static_cast<size_t>(m_screenWidth - 4)) {
            line = line.substr(0, m_screenWidth - 7) + "...";
        }
        if (actualIndex == m_selectedIndex) {
            if (has_colors()) attron(COLOR_PAIR(2));
            mvhline(y, 0, ' ', m_screenWidth);
        }
        if (todo.isCompleted()) {
            if (has_colors()) attron(COLOR_PAIR(3));
        }
        mvprintw(y, 2, "%s", line.c_str());
        if (todo.isCompleted()) attroff(COLOR_PAIR(3));
        if (actualIndex == m_selectedIndex) attroff(COLOR_PAIR(2));
    }
}

void App::showAddDialog() {
    echo();
    curs_set(1);
    nodelay(stdscr, FALSE);
    
    int dialogY = m_screenHeight / 2 - 4;
    int dialogX = (m_screenWidth - 60) / 2;
    
    // Ensure dialog fits on screen
    if (dialogY < 0) dialogY = 0;
    if (dialogX < 0) dialogX = 0;
    
    attron(COLOR_PAIR(1));
    mvhline(dialogY, dialogX, ACS_HLINE, 60);
    mvhline(dialogY + 6, dialogX, ACS_HLINE, 60);
    for (int i = 0; i < 7; i++) {
        mvvline(dialogY + i, dialogX, ACS_VLINE, 1);
        mvvline(dialogY + i, dialogX + 59, ACS_VLINE, 1);
    }
    mvhline(dialogY, dialogX, ACS_ULCORNER, 1);
    mvhline(dialogY, dialogX + 59, ACS_URCORNER, 1);
    mvhline(dialogY + 6, dialogX, ACS_LLCORNER, 1);
    mvhline(dialogY + 6, dialogX + 59, ACS_LRCORNER, 1);
    mvprintw(dialogY, dialogX + 25, " ADD TODO ");
    attroff(COLOR_PAIR(1));
    
    mvprintw(dialogY + 2, dialogX + 2, "Description: ");
    mvprintw(dialogY + 4, dialogX + 2, "Priority (1=Low, 2=Medium, 3=High): ");
    mvprintw(dialogY + 5, dialogX + 2, "Press Enter to save, Esc to cancel");
    refresh();
    
    char desc[256] = {0};
    mvprintw(dialogY + 2, dialogX + 16, "                    ");
    move(dialogY + 2, dialogX + 16);
    getnstr(desc, 255);
    
    if (desc[0] == 27 || strlen(desc) == 0) {
        curs_set(0);
        noecho();
        nodelay(stdscr, TRUE);
        return;
    }
    
    char prioStr[10] = {0};
    mvprintw(dialogY + 4, dialogX + 42, "   ");
    move(dialogY + 4, dialogX + 42);
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
    echo();
    curs_set(1);
    nodelay(stdscr, FALSE);
    
    int dialogY = m_screenHeight / 2 - 4;
    int dialogX = (m_screenWidth - 60) / 2;
    
    if (dialogY < 0) dialogY = 0;
    if (dialogX < 0) dialogX = 0;
    
    attron(COLOR_PAIR(1));
    mvhline(dialogY, dialogX, ACS_HLINE, 60);
    mvhline(dialogY + 6, dialogX, ACS_HLINE, 60);
    for (int i = 0; i < 7; i++) {
        mvvline(dialogY + i, dialogX, ACS_VLINE, 1);
        mvvline(dialogY + i, dialogX + 59, ACS_VLINE, 1);
    }
    mvhline(dialogY, dialogX, ACS_ULCORNER, 1);
    mvhline(dialogY, dialogX + 59, ACS_URCORNER, 1);
    mvhline(dialogY + 6, dialogX, ACS_LLCORNER, 1);
    mvhline(dialogY + 6, dialogX + 59, ACS_LRCORNER, 1);
    mvprintw(dialogY, dialogX + 25, " EDIT TODO ");
    attroff(COLOR_PAIR(1));
    
    mvprintw(dialogY + 2, dialogX + 2, "Description: ");
    mvprintw(dialogY + 3, dialogX + 16, "%s", todo.description.c_str());
    mvprintw(dialogY + 4, dialogX + 2, "Priority (1=Low, 2=Medium, 3=High): ");
    mvprintw(dialogY + 4, dialogX + 42, "%d", static_cast<int>(todo.priority));
    mvprintw(dialogY + 5, dialogX + 2, "Press Enter to save, Esc to cancel");
    refresh();
    
    char desc[256] = {0};
    move(dialogY + 2, dialogX + 16);
    getnstr(desc, 255);
    
    if (desc[0] == 27) {
        curs_set(0);
        noecho();
        nodelay(stdscr, TRUE);
        return;
    }
    
    std::string newDesc = strlen(desc) > 0 ? desc : todo.description;
    char prioStr[10] = {0};
    move(dialogY + 4, dialogX + 42);
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
    echo();
    curs_set(1);
    nodelay(stdscr, FALSE);
    
    int dialogY = m_screenHeight / 2 - 3;
    int dialogX = (m_screenWidth - 50) / 2;
    
    if (dialogY < 0) dialogY = 0;
    if (dialogX < 0) dialogX = 0;
    
    attron(COLOR_PAIR(7));
    mvhline(dialogY, dialogX, ACS_HLINE, 50);
    mvhline(dialogY + 5, dialogX, ACS_HLINE, 50);
    for (int i = 0; i < 6; i++) {
        mvvline(dialogY + i, dialogX, ACS_VLINE, 1);
        mvvline(dialogY + i, dialogX + 49, ACS_VLINE, 1);
    }
    mvhline(dialogY, dialogX, ACS_ULCORNER, 1);
    mvhline(dialogY, dialogX + 49, ACS_URCORNER, 1);
    mvhline(dialogY + 5, dialogX, ACS_LLCORNER, 1);
    mvhline(dialogY + 5, dialogX + 49, ACS_LRCORNER, 1);
    mvprintw(dialogY, dialogX + 15, " DELETE TODO ");
    attroff(COLOR_PAIR(7));
    
    mvprintw(dialogY + 2, dialogX + 2, "Delete: \"%s\"?", todo.description.c_str());
    mvprintw(dialogY + 3, dialogX + 2, "Press 'y' to confirm, any other key to cancel");
    refresh();
    
    int ch = getch();
    if (ch == 'y' || ch == 'Y') {
        m_db.deleteTodo(todo.id);
        if (m_selectedIndex >= static_cast<int>(m_todos.size()) - 1) {
            m_selectedIndex = m_todos.empty() ? 0 : m_todos.size() - 1;
        }
        refreshTodos();
    }
    
    curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);
}

void App::showHelpDialog() {
    echo();
    curs_set(1);
    nodelay(stdscr, FALSE);
    
    int dialogY = m_screenHeight / 2 - 7;
    int dialogX = (m_screenWidth - 60) / 2;
    
    if (dialogY < 0) dialogY = 0;
    if (dialogX < 0) dialogX = 0;
    
    attron(COLOR_PAIR(1));
    mvhline(dialogY, dialogX, ACS_HLINE, 60);
    mvhline(dialogY + 14, dialogX, ACS_HLINE, 60);
    for (int i = 0; i < 15; i++) {
        mvvline(dialogY + i, dialogX, ACS_VLINE, 1);
        mvvline(dialogY + i, dialogX + 59, ACS_VLINE, 1);
    }
    mvhline(dialogY, dialogX, ACS_ULCORNER, 1);
    mvhline(dialogY, dialogX + 59, ACS_URCORNER, 1);
    mvhline(dialogY + 14, dialogX, ACS_LLCORNER, 1);
    mvhline(dialogY + 14, dialogX + 59, ACS_LRCORNER, 1);
    mvprintw(dialogY, dialogX + 25, " HELP ");
    attroff(COLOR_PAIR(1));
    
    const char* helpText[] = {
        "Keyboard Shortcuts:", "",
        "  n     - Add new todo",
        "  e     - Edit selected todo",
        "  d     - Delete selected todo",
        "  t     - Toggle completion",
        "  f     - Filter (All/Active/Completed)",
        "  s     - Search todos",
        "  h/?   - Show this help",
        "  Up/Down - Navigate list",
        "  q/F10 - Quit", "",
        "Press any key to continue..."
    };
    
    for (int i = 0; i < 13; i++) {
        mvprintw(dialogY + 2 + i, dialogX + 3, "%s", helpText[i]);
    }
    refresh();
    getch();
    
    curs_set(0);
    noecho();
    nodelay(stdscr, TRUE);
}

void App::showSearchDialog() {
    echo();
    curs_set(1);
    nodelay(stdscr, FALSE);
    
    int dialogY = m_screenHeight / 2 - 2;
    int dialogX = (m_screenWidth - 40) / 2;
    
    if (dialogY < 0) dialogY = 0;
    if (dialogX < 0) dialogX = 0;
    
    attron(COLOR_PAIR(1));
    mvhline(dialogY, dialogX, ACS_HLINE, 40);
    mvhline(dialogY + 3, dialogX, ACS_HLINE, 40);
    for (int i = 0; i < 4; i++) {
        mvvline(dialogY + i, dialogX, ACS_VLINE, 1);
        mvvline(dialogY + i, dialogX + 39, ACS_VLINE, 1);
    }
    mvhline(dialogY, dialogX, ACS_ULCORNER, 1);
    mvhline(dialogY, dialogX + 39, ACS_URCORNER, 1);
    mvhline(dialogY + 3, dialogX, ACS_LLCORNER, 1);
    mvhline(dialogY + 3, dialogX + 39, ACS_LRCORNER, 1);
    mvprintw(dialogY, dialogX + 12, " SEARCH ");
    attroff(COLOR_PAIR(1));
    
    mvprintw(dialogY + 1, dialogX + 2, "Enter search text: ");
    mvprintw(dialogY + 2, dialogX + 2, "Press Enter to search, Esc to cancel");
    refresh();
    
    char query[128] = {0};
    move(dialogY + 1, dialogX + 20);
    getnstr(query, 127);
    
    if (query[0] != 27) {
        m_searchQuery = query;
        if (!m_searchQuery.empty()) {
            std::vector<todo::Todo> results = m_db.searchTodos(m_searchQuery);
            std::vector<todo::Todo> oldTodos = m_todos;
            m_todos = results;
            m_selectedIndex = 0;
            m_scrollOffset = 0;
            showMainScreen();
            mvprintw(m_screenHeight - 2, 2, "Found %zu results. Press any key to return...", results.size());
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
            m_selectedIndex = m_todos.empty() ? 0 : m_todos.size() - 1;
        }
        refreshTodos();
    }
}

void App::selectTodo(int index) {
    if (index >= 0 && index < static_cast<int>(m_todos.size())) {
        m_selectedIndex = index;
    }
}