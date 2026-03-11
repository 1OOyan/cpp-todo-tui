#ifndef MODELS_HPP
#define MODELS_HPP

#include <string>
#include <chrono>
#include <optional>

namespace todo {

enum class Priority {
    Low = 1,
    Medium = 2,
    High = 3
};

enum class TodoStatus {
    Active = 0,
    Completed = 1
};

struct Todo {
    int id = 0;
    std::string description;
    Priority priority = Priority::Low;
    TodoStatus status = TodoStatus::Active;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
    std::optional<std::chrono::system_clock::time_point> completed_at;

    std::string getPriorityString() const {
        switch (priority) {
            case Priority::Low: return "LOW";
            case Priority::Medium: return "MEDIUM";
            case Priority::High: return "HIGH";
            default: return "UNKNOWN";
        }
    }

    std::string getStatusString() const {
        return status == TodoStatus::Completed ? "[x]" : "[ ]";
    }

    bool isCompleted() const {
        return status == TodoStatus::Completed;
    }
};

} // namespace todo

#endif // MODELS_HPP
