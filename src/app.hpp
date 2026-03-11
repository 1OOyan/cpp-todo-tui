#ifndef APP_HPP
#define APP_HPP

#include <vector>
#include <string>
#include "models.hpp"
#include "database.hpp"

class App {
public:
    App();
    ~App();

    void run();

private:
    void showMainScreen();
    void showTodoList(const std::vector<todo::Todo>& todos);
    void showAddDialog();
    void showEditDialog(const todo::Todo& todo);
    void showDeleteConfirm(const todo::Todo& todo);
    void showSearchDialog();
    void showHelpDialog();
    void showStats();
    
    void renderHeader();
    void renderFooter();
    void renderStatusLine();
    
    todo::Todo inputNewTodo();
    void selectTodo(int index);
    void toggleTodo(int index);
    void deleteTodo(int index);
    void filterTodos();
    void searchTodos();
    void refreshTodos();
    
    std::string m_searchQuery;
    int m_selectedIndex = 0;
    int m_scrollOffset = 0;
    todo::TodoStatus m_currentFilter = todo::TodoStatus::Active;
    
    Database m_db;
    std::vector<todo::Todo> m_todos;
    
    int m_screenWidth = 80;
    int m_screenHeight = 24;
};

#endif // APP_HPP