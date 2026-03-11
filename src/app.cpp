#include "app.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <sstream>
#include <algorithm>
#include <functional>

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
        return vbox({
            renderHeader(),
            separator(),
            renderTodoList(),
            separator(),
            renderStatusLine(),
            separator(),
            renderFooter(),
        }) | border;
    });
    
    auto main_component = CatchEvent(main_ui, [this](Event event) {
        if (event == Event::Character('q')) {
            m_screen.ExitLoopClosure();
            return true;
        }
        
        if (m_showHelp) {
            m_showHelp = false;
            return true;
        }
        
        if (event == Event::Character('n')) {
            addTodo();
            return true;
        }
        if (event == Event::Character('e')) {
            if (!m_todos.empty()) editTodo();
            return true;
        }
        if (event == Event::Character('d')) {
            if (!m_todos.empty()) deleteTodo();
            return true;
        }
        if (event == Event::Character('t')) {
            if (!m_todos.empty()) toggleTodo();
            return true;
        }
        if (event == Event::Character('f')) {
            filterTodos();
            return true;
        }
        if (event == Event::Character('s')) {
            searchTodos();
            return true;
        }
        if (event == Event::Character('h') || event == Event::Character('?')) {
            m_showHelp = true;
            showHelp();
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
        
        return false;
    });
    
    m_screen.Loop(main_component);
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

void App::addTodo() {
    std::string description;
    int selectedPriority = 2;
    bool done = false;
    bool error = false;
    std::string errorMsg;
    
    auto input_desc = Input(&description, "Description:");
    
    while (!done) {
        auto dialog_ui = Renderer([&]() {
            std::string prioText;
            switch (selectedPriority) {
                case 1: prioText = "Low"; break;
                case 2: prioText = "Medium"; break;
                case 3: prioText = "High"; break;
            }
            
            Elements content;
            content.push_back(text("Add New Todo") | bold);
            content.push_back(separator());
            content.push_back(text("Description:"));
            content.push_back(input_desc->Render());
            content.push_back(text(""));
            content.push_back(separator());
            content.push_back(text("Priority (1=Low, 2=Medium, 3=High): ") | dim);
            content.push_back(text(prioText) | bold);
            content.push_back(text("  Press 1, 2, or 3 to change, Enter to save") | dim);
            
            if (error) {
                content.push_back(text(""));
                content.push_back(separator());
                content.push_back(text(errorMsg) | color(Color::Red));
            }
            
            return vbox(content) | border;
        });
        
        auto dialog_comp = CatchEvent(dialog_ui, [&](Event event) {
            if (event == Event::Escape) {
                done = true;
                return true;
            }
            
            if (event == Event::Return) {
                if (description.empty()) {
                    error = true;
                    errorMsg = "Description cannot be empty!";
                    return true;
                }
                
                todo::Todo newTodo;
                newTodo.description = description;
                newTodo.priority = static_cast<todo::Priority>(selectedPriority);
                m_db.createTodo(newTodo);
                refreshTodos();
                done = true;
                return true;
            }
            
            if (event == Event::Character('1')) {
                selectedPriority = 1;
                error = false;
                return true;
            }
            if (event == Event::Character('2')) {
                selectedPriority = 2;
                error = false;
                return true;
            }
            if (event == Event::Character('3')) {
                selectedPriority = 3;
                error = false;
                return true;
            }
            
            return false;
        });
        
        m_screen.Loop(dialog_comp);
    }
}

void App::editTodo() {
    if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_todos.size())) return;
    
    todo::Todo& todo = m_todos[m_selectedIndex];
    std::string description = todo.description;
    int selectedPriority = static_cast<int>(todo.priority);
    bool done = false;
    bool error = false;
    std::string errorMsg;
    
    auto input_desc = Input(&description, "Description:");
    
    while (!done) {
        auto dialog_ui = Renderer([&]() {
            std::string prioText;
            switch (selectedPriority) {
                case 1: prioText = "Low"; break;
                case 2: prioText = "Medium"; break;
                case 3: prioText = "High"; break;
            }
            
            Elements content;
            content.push_back(text("Edit Todo") | bold);
            content.push_back(separator());
            content.push_back(text("Description:"));
            content.push_back(input_desc->Render());
            content.push_back(text(""));
            content.push_back(separator());
            content.push_back(text("Current Priority: ") | dim);
            content.push_back(text(prioText) | bold);
            content.push_back(text("  Press 1, 2, or 3 to change priority") | dim);
            content.push_back(text("  Press Enter to save, Esc to cancel") | dim);
            
            if (error) {
                content.push_back(text(""));
                content.push_back(separator());
                content.push_back(text(errorMsg) | color(Color::Red));
            }
            
            return vbox(content) | border;
        });
        
        auto dialog_comp = CatchEvent(dialog_ui, [&](Event event) {
            if (event == Event::Escape) {
                done = true;
                return true;
            }
            
            if (event == Event::Return) {
                if (description.empty()) {
                    error = true;
                    errorMsg = "Description cannot be empty!";
                    return true;
                }
                
                todo.description = description;
                todo.priority = static_cast<todo::Priority>(selectedPriority);
                m_db.updateTodo(todo);
                refreshTodos();
                done = true;
                return true;
            }
            
            if (event == Event::Character('1')) {
                selectedPriority = 1;
                error = false;
                return true;
            }
            if (event == Event::Character('2')) {
                selectedPriority = 2;
                error = false;
                return true;
            }
            if (event == Event::Character('3')) {
                selectedPriority = 3;
                error = false;
                return true;
            }
            return false;
        });
        
        m_screen.Loop(dialog_comp);
    }
}

void App::deleteTodo() {
    if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_todos.size())) return;
    
    const auto& todo = m_todos[m_selectedIndex];
    bool done = false;
    bool confirmed = false;
    
    while (!done) {
        auto dialog_ui = Renderer([&]() {
            Elements content;
            content.push_back(text("Delete Todo?") | bold | color(Color::Red));
            content.push_back(separator());
            content.push_back(text("\"" + todo.description + "\""));
            content.push_back(text(""));
            content.push_back(separator());
            content.push_back(text("Press Y to confirm, N to cancel") | dim);
            
            return vbox(content) | border;
        });
        
        auto dialog_comp = CatchEvent(dialog_ui, [&](Event event) {
            if (event == Event::Character('y') || event == Event::Character('Y')) {
                m_db.deleteTodo(todo.id);
                if (m_selectedIndex >= static_cast<int>(m_todos.size())) {
                    m_selectedIndex = m_todos.empty() ? 0 : static_cast<int>(m_todos.size()) - 1;
                }
                refreshTodos();
                confirmed = true;
            }
            done = true;
            return true;
        });
        
        m_screen.Loop(dialog_comp);
    }
}

void App::toggleTodo() {
    if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_todos.size())) return;
    
    todo::Todo& todo = m_todos[m_selectedIndex];
    todo.status = todo.isCompleted() ? todo::TodoStatus::Active : todo::TodoStatus::Completed;
    m_db.updateTodo(todo);
    
    if (m_currentFilter == todo::TodoStatus::Active && todo.isCompleted()) {
        if (m_selectedIndex < static_cast<int>(m_todos.size()) - 1) {
            m_selectedIndex++;
        } else if (m_selectedIndex > 0) {
            m_selectedIndex--;
        }
    } else if (m_currentFilter == todo::TodoStatus::Completed && !todo.isCompleted()) {
        if (m_selectedIndex < static_cast<int>(m_todos.size()) - 1) {
            m_selectedIndex++;
        } else if (m_selectedIndex > 0) {
            m_selectedIndex--;
        }
    }
    
    refreshTodos();
}

void App::filterTodos() {
    if (m_currentFilter == todo::TodoStatus::Active) {
        m_currentFilter = todo::TodoStatus::Completed;
    } else {
        m_currentFilter = todo::TodoStatus::Active;
    }
    
    m_searchQuery.clear();
    refreshTodos();
}

void App::searchTodos() {
    std::string query;
    bool done = false;
    
    auto input_query = Input(&query, "Search:");
    
    while (!done) {
        auto dialog_ui = Renderer([&]() {
            Elements content;
            content.push_back(text("Search Todos") | bold);
            content.push_back(separator());
            content.push_back(text("Enter text to search:"));
            content.push_back(input_query->Render());
            content.push_back(text(""));
            content.push_back(separator());
            content.push_back(text("Press Enter to search, Esc to cancel") | dim);
            
            return vbox(content) | border;
        });
        
        auto dialog_comp = CatchEvent(dialog_ui, [&](Event event) {
            if (event == Event::Escape) {
                m_searchQuery.clear();
                refreshTodos();
                done = true;
                return true;
            }
            
            if (event == Event::Return) {
                if (!query.empty()) {
                    m_searchQuery = query;
                    todo::TodoStatus oldFilter = m_currentFilter;
                    m_todos = m_db.searchTodos(m_searchQuery);
                    m_currentFilter = oldFilter;
                    m_selectedIndex = 0;
                } else {
                    m_searchQuery.clear();
                    refreshTodos();
                }
                done = true;
                return true;
            }
            return false;
        });
        
        m_screen.Loop(dialog_comp);
    }
}

void App::showHelp() {
    bool done = false;
    
    while (!done) {
        auto help_ui = Renderer([&]() {
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
            
            return vbox(content) | border;
        });
        
        auto help_comp = CatchEvent(help_ui, [&](Event) {
            done = true;
            m_showHelp = false;
            return true;
        });
        
        m_screen.Loop(help_comp);
    }
}