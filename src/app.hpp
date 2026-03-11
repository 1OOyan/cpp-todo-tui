#ifndef APP_HPP
#define APP_HPP

#include <vector>
#include <string>
#include <memory>
#include "models.hpp"
#include "database.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>

class App {
public:
    App();
    ~App();

    void run();

private:
    void showMainScreen();
    ftxui::Element renderTodoList();
    ftxui::Element renderHeader();
    ftxui::Element renderFooter();
    ftxui::Element renderStatusLine();
    
    void addTodo();
    void editTodo();
    void deleteTodo();
    void toggleTodo();
    void filterTodos();
    void searchTodos();
    void refreshTodos();
    void showHelp();
    
    // State
    std::string m_searchQuery;
    int m_selectedIndex = 0;
    todo::TodoStatus m_currentFilter;
    
    // FTXUI
    ftxui::ScreenInteractive m_screen;
    Database m_db;
    std::vector<todo::Todo> m_todos;
    
    // Dialog state
    bool m_showHelp = false;
    std::string m_dialogInput;
    int m_dialogPriority = 1;
    todo::Todo m_currentTodo;
};

#endif // APP_HPP