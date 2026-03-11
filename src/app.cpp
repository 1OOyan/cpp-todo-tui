#include "app.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <sstream>
#include <algorithm>

using namespace ftxui;

App::App() : m_currentFilter(todo::TodoStatus::Active), m_screen(ScreenInteractive::Fullscreen()), m_db("todo.db") {
    refreshTodos();
}

App::~App() = default;

void App::refreshTodos() {
    if (m_currentFilter == todo::TodoStatus::Active) {
        m_todos = m_db.getTodosByStatus(todo::TodoStatus::Active);
    } else {
        m_todos = m_db.getTodosByStatus(todo::TodoStatus::Completed);
    }
    
    if (m_todos.empty()) {
        m_selectedIndex = 0;
    } else if (m_selectedIndex >= static_cast<int>(m_todos.size())) {
        m_selectedIndex = static_cast<int>(m_todos.size()) - 1;
    }
    if (m_selectedIndex < 0) m_selectedIndex = 0;
}

void App::run() {
    auto main_ui = Renderer([this]() {
        if (m_dialogState == DialogState::None) {
            return renderMainView();
        } else if (m_dialogState == DialogState::Add) {
            return renderAddDialog();
        } else if (m_dialogState == DialogState::Edit) {
            return renderEditDialog();
        } else if (m_dialogState == DialogState::Delete) {
            return renderDeleteDialog();
        } else if (m_dialogState == DialogState::Search) {
            return renderSearchDialog();
        } else if (m_dialogState == DialogState::Help) {
            return renderHelpDialog();
        }
        return renderMainView();
    });
    
    auto main_component = CatchEvent(main_ui, [this](Event event) {
        // Global quit (only in main view)
        if (event == Event::Character('q') && m_dialogState == DialogState::None) {
            m_screen.ExitLoopClosure();
            return true;
        }
        
        // ESC always cancels dialogs
        if (event == Event::Escape && m_dialogState != DialogState::None) {
            m_dialogState = DialogState::None;
            m_dialogError.clear();
            m_dialogTodoIndex = -1;
            return true;
        }
        
        // Handle dialog-specific events
        if (m_dialogState == DialogState::Add) {
            if (event == Event::Return) {
                if (m_dialogDescription.empty()) {
                    m_dialogError = "Description cannot be empty!";
                    return true;
                }
                todo::Todo newTodo;
                newTodo.description = m_dialogDescription;
                newTodo.priority = static_cast<todo::Priority>(m_dialogPriority);
                m_db.createTodo(newTodo);
                refreshTodos();
                m_dialogState = DialogState::None;
                m_dialogDescription.clear();
                m_dialogPriority = 2;
                m_dialogError.clear();
                return true;
            }
            if (event == Event::Character('1')) {
                m_dialogPriority = 1;
                m_dialogError.clear();
                return true;
            }
            if (event == Event::Character('2')) {
                m_dialogPriority = 2;
                m_dialogError.clear();
                return true;
            }
            if (event == Event::Character('3')) {
                m_dialogPriority = 3;
                m_dialogError.clear();
                return true;
            }
            // Handle text input
            if (event.is_character()) {
                m_dialogDescription += event.character();
                m_dialogError.clear();
                return true;
            }
            if (event == Event::Backspace) {
                if (!m_dialogDescription.empty()) {
                    m_dialogDescription.pop_back();
                }
                return true;
            }
            return false;
        }
        
        if (m_dialogState == DialogState::Edit) {
            if (event == Event::Return) {
                if (m_dialogDescription.empty()) {
                    m_dialogError = "Description cannot be empty!";
                    return true;
                }
                if (m_dialogTodoIndex >= 0 && m_dialogTodoIndex < static_cast<int>(m_todos.size())) {
                    m_todos[m_dialogTodoIndex].description = m_dialogDescription;
                    m_todos[m_dialogTodoIndex].priority = static_cast<todo::Priority>(m_dialogPriority);
                    m_db.updateTodo(m_todos[m_dialogTodoIndex]);
                    refreshTodos();
                }
                m_dialogState = DialogState::None;
                m_dialogDescription.clear();
                m_dialogTodoIndex = -1;
                m_dialogError.clear();
                return true;
            }
            if (event == Event::Character('1')) {
                m_dialogPriority = 1;
                m_dialogError.clear();
                return true;
            }
            if (event == Event::Character('2')) {
                m_dialogPriority = 2;
                m_dialogError.clear();
                return true;
            }
            if (event == Event::Character('3')) {
                m_dialogPriority = 3;
                m_dialogError.clear();
                return true;
            }
            // Handle text input
            if (event.is_character()) {
                m_dialogDescription += event.character();
                m_dialogError.clear();
                return true;
            }
            if (event == Event::Backspace) {
                if (!m_dialogDescription.empty()) {
                    m_dialogDescription.pop_back();
                }
                return true;
            }
            return false;
        }
        
        if (m_dialogState == DialogState::Delete) {
            if (event == Event::Character('y') || event == Event::Character('Y')) {
                if (m_dialogTodoIndex >= 0 && m_dialogTodoIndex < static_cast<int>(m_todos.size())) {
                    m_db.deleteTodo(m_todos[m_dialogTodoIndex].id);
                    if (m_selectedIndex >= static_cast<int>(m_todos.size())) {
                        m_selectedIndex = m_todos.empty() ? 0 : static_cast<int>(m_todos.size()) - 1;
                    }
                    refreshTodos();
                }
            }
            m_dialogState = DialogState::None;
            m_dialogTodoIndex = -1;
            return true;
        }
        
        if (m_dialogState == DialogState::Search) {
            if (event == Event::Return) {
                if (!m_dialogSearchQuery.empty()) {
                    m_searchQuery = m_dialogSearchQuery;
                    m_todos = m_db.searchTodos(m_searchQuery);
                    m_selectedIndex = 0;
                } else {
                    m_searchQuery.clear();
                    refreshTodos();
                }
                m_dialogState = DialogState::None;
                m_dialogSearchQuery.clear();
                return true;
            }
            // Handle text input
            if (event.is_character()) {
                m_dialogSearchQuery += event.character();
                return true;
            }
            if (event == Event::Backspace) {
                if (!m_dialogSearchQuery.empty()) {
                    m_dialogSearchQuery.pop_back();
                }
                return true;
            }
            return false;
        }
        
        if (m_dialogState == DialogState::Help) {
            m_dialogState = DialogState::None;
            return true;
        }
        
        // Main view events (only when no dialog)
        if (m_dialogState == DialogState::None) {
            if (event == Event::Character('n')) {
                m_dialogState = DialogState::Add;
                m_dialogDescription.clear();
                m_dialogPriority = 2;
                m_dialogError.clear();
                return true;
            }
            if (event == Event::Character('e')) {
                if (!m_todos.empty()) {
                    m_dialogState = DialogState::Edit;
                    m_dialogTodoIndex = m_selectedIndex;
                    m_dialogDescription = m_todos[m_selectedIndex].description;
                    m_dialogPriority = static_cast<int>(m_todos[m_selectedIndex].priority);
                    m_dialogError.clear();
                }
                return true;
            }
            if (event == Event::Character('d')) {
                if (!m_todos.empty()) {
                    m_dialogState = DialogState::Delete;
                    m_dialogTodoIndex = m_selectedIndex;
                }
                return true;
            }
            if (event == Event::Character('t')) {
                if (!m_todos.empty()) {
                    todo::Todo& todo = m_todos[m_selectedIndex];
                    todo.status = todo.isCompleted() ? todo::TodoStatus::Active : todo::TodoStatus::Completed;
                    m_db.updateTodo(todo);
                    refreshTodos();
                }
                return true;
            }
            if (event == Event::Character('f')) {
                if (m_currentFilter == todo::TodoStatus::Active) {
                    m_currentFilter = todo::TodoStatus::Completed;
                } else {
                    m_currentFilter = todo::TodoStatus::Active;
                }
                m_searchQuery.clear();
                refreshTodos();
                return true;
            }
            if (event == Event::Character('s')) {
                m_dialogState = DialogState::Search;
                m_dialogSearchQuery = m_searchQuery;
                return true;
            }
            if (event == Event::Character('h') || event == Event::Character('?')) {
                m_dialogState = DialogState::Help;
                return true;
            }
            if (event == Event::ArrowUp || event == Event::Character('k')) {
                if (m_selectedIndex > 0) m_selectedIndex--;
                return true;
            }
            if (event == Event::ArrowDown || event == Event::Character('j')) {
                if (m_selectedIndex < static_cast<int>(m_todos.size()) - 1) m_selectedIndex++;
                return true;
            }
        }
        
        return false;
    });
    
    m_screen.Loop(main_component);
}

ftxui::Element App::renderMainView() {
    return vbox({
        renderHeader(),
        separator(),
        renderTodoList(),
        separator(),
        renderStatusLine(),
        separator(),
        renderFooter(),
    }) | border;
}

ftxui::Element App::renderHeader() {
    return hbox({
        filler(),
        text("  TODO LIST  ") | bold | color(Color::White),
        filler()
    }) | bgcolor(Color::Blue) | color(Color::White);
}

ftxui::Element App::renderFooter() {
    return hbox({
        text("[n]New ") | dim,
        text("[e]Edit ") | dim,
        text("[d]Delete ") | dim,
        text("[t]Toggle ") | dim,
        text("[f]Filter ") | dim,
        text("[s]Search ") | dim,
        text("[h]Help ") | dim,
        text("[q]Quit") | dim
    }) | bgcolor(Color::Blue) | color(Color::White);
}

ftxui::Element App::renderStatusLine() {
    int total = m_db.getTotalCount();
    int active = m_db.getActiveCount();
    int completed = m_db.getCompletedCount();
    
    std::stringstream ss;
    ss << " Total: " << total << " | Active: " << active << " | Done: " << completed;
    
    if (m_currentFilter == todo::TodoStatus::Active) {
        ss << " | Filter: Active";
    } else {
        ss << " | Filter: Completed";
    }
    
    if (!m_searchQuery.empty()) {
        ss << " | Search: \"" << m_searchQuery << "\"";
    }
    
    return text(ss.str()) | dim;
}

ftxui::Element App::renderTodoList() {
    if (m_todos.empty()) {
        return vbox({
            filler(),
            text("📝 No todos yet! Press [n] to add your first todo.") | center | color(Color::Magenta),
            filler()
        });
    }
    
    Elements todoElements;
    int visibleCount = std::min(15, static_cast<int>(m_todos.size()));
    int startIdx = std::max(0, m_selectedIndex - visibleCount / 2);
    startIdx = std::min(startIdx, static_cast<int>(m_todos.size()) - visibleCount);
    
    for (int i = startIdx; i < startIdx + visibleCount && i < static_cast<int>(m_todos.size()); i++) {
        const auto& todo = m_todos[i];
        bool isSelected = (i == m_selectedIndex);
        
        std::string status = todo.isCompleted() ? "[x]" : "[ ]";
        Element statusElem = text(" " + status + " ");
        if (todo.isCompleted()) {
            statusElem = statusElem | color(Color::Green);
        }
        
        std::string prio;
        Color prioColor;
        switch (todo.priority) {
            case todo::Priority::High:
                prio = "[H]";
                prioColor = Color::Yellow;
                break;
            case todo::Priority::Medium:
                prio = "[M]";
                prioColor = Color::Cyan;
                break;
            case todo::Priority::Low:
                prio = "[L]";
                prioColor = Color::Magenta;
                break;
        }
        Element prioElem = text(" " + prio + " ") | color(prioColor);
        
        std::string desc = todo.description;
        if (desc.length() > 60) {
            desc = desc.substr(0, 57) + "...";
        }
        Element descElem = text(" " + desc);
        if (todo.isCompleted()) {
            descElem = descElem | dim | strikethrough;
        }
        
        Element row = hbox({
            statusElem,
            prioElem,
            flex_grow(descElem)
        });
        
        if (isSelected) {
            row = row | inverted | bold;
        }
        
        todoElements.push_back(row);
    }
    
    return vbox(todoElements) | flex;
}

// FIXED: window() is now used correctly as a wrapper function, not a decorator
ftxui::Element App::renderAddDialog() {
    std::string prioText;
    switch (m_dialogPriority) {
        case 1: prioText = "Low"; break;
        case 2: prioText = "Medium"; break;
        case 3: prioText = "High"; break;
    }
    
    std::string inputText = m_dialogDescription;
    if (inputText.empty()) {
        inputText = "Type description here...";
    }
    
    Elements content;
    content.push_back(text("Description:"));
    content.push_back(text("┌" + std::string(50, '─') + "┐"));
    content.push_back(text("│ " + inputText + std::string(50 - inputText.length(), ' ') + "│") | color(Color::White));
    content.push_back(text("└" + std::string(50, '─') + "┘"));
    content.push_back(text(""));
    content.push_back(separator());
    content.push_back(text("Priority (1=Low, 2=Medium, 3=High): ") | dim);
    content.push_back(text(prioText) | bold);
    content.push_back(text("  Type text, 1/2/3 for priority, Enter to save, Esc to cancel") | dim);
    
    if (!m_dialogError.empty()) {
        content.push_back(text(""));
        content.push_back(separator());
        content.push_back(text(m_dialogError) | color(Color::Red));
    }
    
    // FIXED: window() wraps the content correctly
    return window(text("Add New Todo"), vbox(content) | border) | center;
}

ftxui::Element App::renderEditDialog() {
    std::string prioText;
    switch (m_dialogPriority) {
        case 1: prioText = "Low"; break;
        case 2: prioText = "Medium"; break;
        case 3: prioText = "High"; break;
    }
    
    std::string inputText = m_dialogDescription;
    if (inputText.empty()) {
        inputText = "Type description here...";
    }
    
    Elements content;
    content.push_back(text("Description:"));
    content.push_back(text("┌" + std::string(50, '─') + "┐"));
    content.push_back(text("│ " + inputText + std::string(50 - inputText.length(), ' ') + "│") | color(Color::White));
    content.push_back(text("└" + std::string(50, '─') + "┘"));
    content.push_back(text(""));
    content.push_back(separator());
    content.push_back(text("Current Priority: ") | dim);
    content.push_back(text(prioText) | bold);
    content.push_back(text("  Type text, 1/2/3 for priority, Enter to save, Esc to cancel") | dim);
    
    if (!m_dialogError.empty()) {
        content.push_back(text(""));
        content.push_back(separator());
        content.push_back(text(m_dialogError) | color(Color::Red));
    }
    
    // FIXED: window() wraps the content correctly
    return window(text("Edit Todo"), vbox(content) | border) | center;
}

ftxui::Element App::renderDeleteDialog() {
    Elements content;
    content.push_back(text("Delete Todo?") | bold | color(Color::Red));
    content.push_back(separator());
    if (m_dialogTodoIndex >= 0 && m_dialogTodoIndex < static_cast<int>(m_todos.size())) {
        std::string desc = m_todos[m_dialogTodoIndex].description;
        if (desc.length() > 40) {
            desc = desc.substr(0, 37) + "...";
        }
        content.push_back(text("\"" + desc + "\""));
    }
    content.push_back(text(""));
    content.push_back(separator());
    content.push_back(text("Press Y to confirm, Esc to cancel") | dim);
    
    // FIXED: window() wraps the content correctly
    return window(text("Delete"), vbox(content) | border) | center;
}

ftxui::Element App::renderSearchDialog() {
    std::string inputText = m_dialogSearchQuery;
    if (inputText.empty()) {
        inputText = "Type search text...";
    }
    
    Elements content;
    content.push_back(text("Enter text to search:"));
    content.push_back(text("┌" + std::string(50, '─') + "┐"));
    content.push_back(text("│ " + inputText + std::string(50 - inputText.length(), ' ') + "│") | color(Color::White));
    content.push_back(text("└" + std::string(50, '─') + "┘"));
    content.push_back(text(""));
    content.push_back(separator());
    content.push_back(text("Type to search, Enter to confirm, Esc to cancel") | dim);
    
    // FIXED: window() wraps the content correctly
    return window(text("Search"), vbox(content) | border) | center;
}

ftxui::Element App::renderHelpDialog() {
    Elements content;
    content.push_back(text("  HELP - KEYBOARD SHORTCUTS  ") | bold);
    content.push_back(separator());
    content.push_back(text("  n     - Add new todo"));
    content.push_back(text("  e     - Edit selected todo"));
    content.push_back(text("  d     - Delete selected todo"));
    content.push_back(text("  t     - Toggle completion status"));
    content.push_back(text("  f     - Filter (Active ↔ Completed)"));
    content.push_back(text("  s     - Search todos"));
    content.push_back(text("  ↑/↓   - Navigate list"));
    content.push_back(text("  h/?   - Show this help"));
    content.push_back(text("  q     - Quit application"));
    content.push_back(text(""));
    content.push_back(separator());
    content.push_back(text("  Priority: [H]=High [M]=Medium [L]=Low") | dim);
    content.push_back(text("  Press any key to continue...") | dim);
    
    // FIXED: window() wraps the content correctly
    return window(text("Help"), vbox(content) | border) | center;
}

void App::addTodo() {}
void App::editTodo() {}
void App::deleteTodo() {}
void App::toggleTodo() {}
void App::filterTodos() {}
void App::searchTodos() {}
void App::showHelp() {}