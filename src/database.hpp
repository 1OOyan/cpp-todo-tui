#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <vector>
#include <string>
#include <memory>
#include <optional>
#include "models.hpp"
#include <sqlite3.h>

class Database {
public:
    Database(const std::string& dbPath = "todo.db");
    ~Database();

    // CRUD operations
    bool createTodo(const todo::Todo& todo);
    bool updateTodo(const todo::Todo& todo);
    bool deleteTodo(int id);
    std::optional<todo::Todo> getTodo(int id);
    std::vector<todo::Todo> getAllTodos();
    std::vector<todo::Todo> getTodosByStatus(todo::TodoStatus status);
    std::vector<todo::Todo> searchTodos(const std::string& query);

    // Statistics
    int getTotalCount();
    int getActiveCount();
    int getCompletedCount();

private:
    void initSchema();
    std::string m_dbPath;
    sqlite3* m_db; // SQLite database handle
};

#endif // DATABASE_HPP