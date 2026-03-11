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
    } else if (m_currentFilter == todo::TodoStatus::Completed) {
        m_todos = m_db.getTodosByStatus(todo::TodoStatus::Completed);
    } else {
        m_todos = m_db.getAllTodos();
    }
    
    // Ensure selected index is valid
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
        // Global quit (only in main view, not in dialogs)
        if (event == Event::Character('q')) {
            m_screen.ExitLoopClosure();
            return true;
        }
        
        if (m_showHelp) {
            m_showHelp = false;
            return true;
        }
        
        // Only accept these keys in main view
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
    
    switch (m_currentFilter) {
        case todo::TodoStatus::Active:
            ss << " | Filter: Active";
            break;
        case todo::TodoStatus::Completed:
            ss << " | Filter: Completed";
            break;
        default:
            ss << " | Filter: All";
            break;
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
        
        // Status
        std::string status = todo.isCompleted() ? "[x]" : "[ ]";
        Element statusElem = text(" " + status + " ");
        if (todo.isCompleted()) {
            statusElem = statusElem | color(Color::Green);
        }
        
        // Priority with better formatting
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
        
        // Description
        std::string desc = todo.description;
        // Truncate if too long
        if (desc.length() > 60) {
            desc = desc.substr(0, 57) + "...";
        }
        Element descElem = text(" " + desc);
        if (todo.isCompleted()) {
            descElem = descElem | dim | strikethrough;
        }
        
        // Combine
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
    int selectedPriority = 2; // Medium priority by default
    bool done = false;
    bool error = false;
    std::string errorMsg;
    
    // Create input component with proper focus
    auto input_desc = Input(&description, "Description:");
    
    auto add_ui = Renderer([this, &description, &selectedPriority, &input_desc, &error, &errorMsg]() {
        std::string prioText;
        switch (selectedPriority) {
            case 1: prioText = "Low"; break;
            case 2: prioText = "Medium"; break;
            case 3: prioText = "High"; break;
        }
        
        Elements content = {
            text("Add New Todo") | bold,
            separator(),
            text("Description:"),
            input_desc->Render(),
            text("") | separator,
            text("Priority (1=Low, 2=Medium, 3=High): ") | dim,
            text(prioText) | bold,
            text("  Press 1, 2, or 3 to change, Enter to save") | dim,
        };
        
        if (error) {
            content.push_back(text("") | separator);
            content.push_back(text(errorMsg) | color(Color::Red));
        }
        
        return vbox(content) | border;
    });
    
    auto add_comp = CatchEvent(Animator(input_desc->Render()), [&](Event event) {
        // Handle ESC to cancel
        if (event == Event::Escape) {
            done = true;
            return true;
        }
        
        // Handle Enter to confirm
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
        
        // Priority selection
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
        
        // Let Input component handle its own events
        return false;
    });
    
    // Use Modal to properly handle the dialog
    auto dialog = Modal(add_comp, " ");
    
    while (!done) {
        m_screen.Loop(dialog);
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
    
    auto edit_ui = Renderer([this, &description, &selectedPriority, &input_desc, &error, &errorMsg]() {
        std::string prioText;
        switch (selectedPriority) {
            case 1: prioText = "Low"; break;
            case 2: prioText = "Medium"; break;
            case 3: prioText = "High"; break;
        }
        
        Elements content = {
            text("Edit Todo") | bold,
            separator(),
            text("Description:"),
            input_desc->Render(),
            text("") | separator,
            text("Current Priority: ") | dim,
            text(prioText) | bold,
            text("  Press 1, 2, or 3 to change priority") | dim,
            text("  Press Enter to save, Esc to cancel") | dim,
        };
        
        if (error) {
            content.push_back(text("") | separator);
            content.push_back(text(errorMsg) | color(Color::Red));
        }
        
        return vbox(content) | border;
    });
    
    auto edit_comp = CatchEvent(edit_ui, [&](Event event) {
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
        
        // Priority selection
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
    
    auto dialog = Modal(edit_comp, " ");
    
    while (!done) {
        m_screen.Loop(dialog);
    }
}

void App::deleteTodo() {
    if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_todos.size())) return;
    
    const auto& todo = m_todos[m_selectedIndex];
    bool done = false;
    
    auto confirm_ui = Renderer([&]() {
        return vbox({
            text("Delete Todo?") | bold | color(Color::Red),
            separator(),
            text("\"" + todo.description + "\""),
            text("") | separator,
            text("Press Y to confirm, N to cancel") | dim,
        }) | border;
    });
    
    auto confirm_comp = CatchEvent(confirm_ui, [&](Event event) {
        if (event == Event::Character('y') || event == Event::Character('Y')) {
            m_db.deleteTodo(todo.id);
            if (m_selectedIndex >= static_cast<int>(m_todos.size())) {
                m_selectedIndex = m_todos.empty() ? 0 : static_cast<int>(m_todos.size()) - 1;
            }
            refreshTodos();
        }
        done = true;
        return true;
    });
    
    auto dialog = Modal(confirm_comp, " ");
    
    while (!done) {
        m_screen.Loop(dialog);
    }
}

void App::toggleTodo() {
    if (m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_todos.size())) return;
    
    todo::Todo& todo = m_todos[m_selectedIndex];
    todo.status = todo.isCompleted() ? todo::TodoStatus::Active : todo::TodoStatus::Completed;
    m_db.updateTodo(todo);
    
    // Move to next item if current is now filtered out
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
    switch (m_currentFilter) {
        case todo::TodoStatus::Active:
            m_currentFilter = todo::TodoStatus::Completed;
            break;
        case todo::TodoStatus::Completed:
            m_currentFilter = todo::TodoStatus::All;
            break;
        default:
            m_currentFilter = todo::TodoStatus::Active;
            break;
    }
    
    // Clear search when changing filter
    m_searchQuery.clear();
    refreshTodos();
}

void App::searchTodos() {
    std::string query;
    bool done = false;
    
    auto input_query = Input(&query, "Search:");
    
    auto search_ui = Renderer([&]() {
        return vbox({
            text("Search Todos") | bold,
            separator(),
            text("Enter text to search:"),
            input_query->Render(),
            text("") | separator,
            text("Press Enter to search, Esc to cancel") | dim,
        }) | border;
    });
    
    auto search_comp = CatchEvent(search_ui, [&](Event event) {
        if (event == Event::Escape) {
            m_searchQuery.clear();
            refreshTodos();
            done = true;
            return true;
        }
        
        if (event == Event::Return) {
            if (!query.empty()) {
                m_searchQuery = query;
                // Clear filter when searching
                todo::TodoStatus oldFilter = m_currentFilter;
                m_currentFilter = todo::TodoStatus::All;
                m_todos = m_db.searchTodos(m_searchQuery);
                m_currentFilter = oldFilter;
                m_selectedIndex = 0;
            } else {
                // Empty search clears search
                m_searchQuery.clear();
                refreshTodos();
            }
            done = true;
            return true;
        }
        return false;
    });
    
    auto dialog = Modal(search_comp, " ");
    
    while (!done) {
        m_screen.Loop(dialog);
    }
}

void App::showHelp() {
    bool done = false;
    
    auto help_ui = Renderer([&]() {
        return vbox({
            text("  HELP - KEYBOARD SHORTCUTS  ") | bold,
            separator(),
            text("  n     - Add new todo"),
            text("  e     - Edit selected todo"),
            text("  d     - Delete selected todo"),
            text("  t     - Toggle completion status"),
            text("  f     - Filter (Active → Completed → All)"),
            text("  s     - Search todos"),
            text("  ↑/↓   - Navigate list"),
            text("  h/?   - Show this help"),
            text("  q     - Quit application"),
            separator(),
            text("  Priority: [H]=High [M]=Medium [L]=Low") | dim,
            text("  Press any key to continue...") | dim
        }) | border;
    });
    
    auto help_comp = CatchEvent(help_ui, [&](Event) {
        done = true;
        m_showHelp = false;
        return true;
    });
    
    auto dialog = Modal(help_comp, " ");
    
    while (!done) {
        m_screen.Loop(dialog);
    }
}
