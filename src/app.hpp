#ifndef APP_HPP
#define APP_HPP

#include "database.hpp"
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <vector>
#include <string>

class App {
public:
    App();
    ~App();
    
    void run();
    
private:
    void refreshTodos();
    
    // Rendering
    ftxui::Element renderHeader();
    ftxui::Element renderFooter();
    ftxui::Element renderStatusLine();
    ftxui::Element renderTodoList();
    
    // Actions
    void addTodo();
    void editTodo();
    void deleteTodo();
    void toggleTodo();
    void filterTodos();
    void searchTodos();
    void showHelp();
    
    // State
    int m_selectedIndex = 0;
    todo::TodoStatus m_currentFilter;
    bool m_showHelp = false;
    std::string m_searchQuery;
    
    // Components
    ftxui::ScreenInteractive m_screen;
    Database m_db;
    
    // Data
    std::vector<todo::Todo> m_todos;
};

#endif // APP_HPP