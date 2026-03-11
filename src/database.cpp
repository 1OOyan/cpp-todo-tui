#include "database.hpp"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <iomanip>

Database::Database(const std::string& dbPath) : m_dbPath(dbPath), m_db(nullptr) {
    int rc = sqlite3_open(dbPath.c_str(), &m_db);
    if (rc) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(m_db) << std::endl;
        return;
    }
    initSchema();
}

Database::~Database() {
    if (m_db) {
        sqlite3_close(m_db);
    }
}

void Database::initSchema() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS todos (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            description TEXT NOT NULL,
            priority INTEGER NOT NULL DEFAULT 1,
            status INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
            completed_at TEXT
        );
    )";
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

bool Database::createTodo(const todo::Todo& todo) {
    const char* sql = "INSERT INTO todos (description, priority, status, created_at, updated_at) VALUES (?, ?, ?, datetime('now'), datetime('now'));";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, todo.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, static_cast<int>(todo.priority));
    sqlite3_bind_int(stmt, 3, static_cast<int>(todo.status));
    
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return true;
    }
    
    std::cerr << "Failed to insert todo: " << sqlite3_errmsg(m_db) << std::endl;
    sqlite3_finalize(stmt);
    return false;
}

bool Database::updateTodo(const todo::Todo& todo) {
    const char* sql = "UPDATE todos SET description = ?, priority = ?, status = ?, updated_at = datetime('now'), completed_at = CASE WHEN ? = 1 THEN datetime('now') ELSE completed_at END WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, todo.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, static_cast<int>(todo.priority));
    sqlite3_bind_int(stmt, 3, static_cast<int>(todo.status));
    sqlite3_bind_int(stmt, 4, static_cast<int>(todo.status));
    sqlite3_bind_int(stmt, 5, todo.id);
    
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return true;
    }
    
    std::cerr << "Failed to update todo: " << sqlite3_errmsg(m_db) << std::endl;
    sqlite3_finalize(stmt);
    return false;
}

bool Database::deleteTodo(int id) {
    const char* sql = "DELETE FROM todos WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return sqlite3_changes(m_db) > 0;
    }
    
    std::cerr << "Failed to delete todo: " << sqlite3_errmsg(m_db) << std::endl;
    sqlite3_finalize(stmt);
    return false;
}

std::optional<todo::Todo> Database::getTodo(int id) {
    const char* sql = "SELECT id, description, priority, status, created_at, updated_at, completed_at FROM todos WHERE id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return std::nullopt;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        todo::Todo todo;
        todo.id = sqlite3_column_int(stmt, 0);
        todo.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        todo.priority = static_cast<todo::Priority>(sqlite3_column_int(stmt, 2));
        todo.status = static_cast<todo::TodoStatus>(sqlite3_column_int(stmt, 3));
        
        sqlite3_finalize(stmt);
        return todo;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<todo::Todo> Database::getAllTodos() {
    std::vector<todo::Todo> todos;
    const char* sql = "SELECT id, description, priority, status, created_at, updated_at, completed_at FROM todos ORDER BY priority DESC, created_at ASC;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return todos;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        todo::Todo todo;
        todo.id = sqlite3_column_int(stmt, 0);
        todo.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        todo.priority = static_cast<todo::Priority>(sqlite3_column_int(stmt, 2));
        todo.status = static_cast<todo::TodoStatus>(sqlite3_column_int(stmt, 3));
        todos.push_back(todo);
    }
    
    sqlite3_finalize(stmt);
    return todos;
}

std::vector<todo::Todo> Database::getTodosByStatus(todo::TodoStatus status) {
    std::vector<todo::Todo> todos;
    std::stringstream sql;
    sql << "SELECT id, description, priority, status, created_at, updated_at, completed_at FROM todos WHERE status = " << static_cast<int>(status) << " ORDER BY priority DESC, created_at ASC;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return todos;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        todo::Todo todo;
        todo.id = sqlite3_column_int(stmt, 0);
        todo.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        todo.priority = static_cast<todo::Priority>(sqlite3_column_int(stmt, 2));
        todo.status = static_cast<todo::TodoStatus>(sqlite3_column_int(stmt, 3));
        todos.push_back(todo);
    }
    
    sqlite3_finalize(stmt);
    return todos;
}

std::vector<todo::Todo> Database::searchTodos(const std::string& query) {
    std::vector<todo::Todo> todos;
    std::stringstream sql;
    sql << "SELECT id, description, priority, status, created_at, updated_at, completed_at FROM todos WHERE description LIKE '%" << query << "%' ORDER BY priority DESC, created_at ASC;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(m_db, sql.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;
        return todos;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        todo::Todo todo;
        todo.id = sqlite3_column_int(stmt, 0);
        todo.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        todo.priority = static_cast<todo::Priority>(sqlite3_column_int(stmt, 2));
        todo.status = static_cast<todo::TodoStatus>(sqlite3_column_int(stmt, 3));
        todos.push_back(todo);
    }
    
    sqlite3_finalize(stmt);
    return todos;
}

int Database::getTotalCount() {
    const char* sql = "SELECT COUNT(*) FROM todos;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

int Database::getActiveCount() {
    const char* sql = "SELECT COUNT(*) FROM todos WHERE status = 0;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

int Database::getCompletedCount() {
    const char* sql = "SELECT COUNT(*) FROM todos WHERE status = 1;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}