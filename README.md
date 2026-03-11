# C++ Todo TUI Application

A beautiful **Terminal User Interface (TUI)** Todo application built with modern C++17 and **FTXUI**, featuring:

- 🎨 **Modern TUI** using FTXUI - a functional C++ TUI library inspired by React
- 💾 **SQLite persistence** for reliable data storage  
- ⚡ **Fast and responsive** with modern C++17 features
- 🖱️ **Mouse and keyboard support** for intuitive interaction
- 🌈 **Beautiful colors** and styling with full UTF-8 support

## Features

- ✅ Add new todos with priority levels (Low, Medium, High)
- 📋 View all todos with filtering options
- ✏️ Edit existing todos
- 🗑️ Delete completed or unwanted todos
- 🎯 Filter by status (All, Active, Completed)
- 🔍 Search through todos
- 📊 View statistics and progress
- 💾 Automatic save to SQLite database
- 🎨 Beautiful UI with colors, borders, and layouts

## Why FTXUI?

**FTXUI (Functional Terminal User Interface)** is a modern C++ TUI library that:
- Uses a **functional/reactive** style inspired by React
- Provides beautiful **widgets and layouts** (vbox, hbox, grid, etc.)
- Supports **colors, borders, animations**, and **UTF-8**
- Has **no dependencies** - pure C++
- Is **cross-platform** (Linux, macOS, Windows, WebAssembly)
- Offers **keyboard and mouse** interaction

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libsqlite3-dev
```

### macOS
```bash
brew install cmake sqlite3
```

### Windows (with MSVC)
```powershell
# Install vcpkg and required packages
vcpkg install sqlite3
```

## Building

### Using CMake with FetchContent (Recommended)

```bash
# Clone the repository
git clone https://github.com/1OOyan/cpp-todo-tui.git
cd cpp-todo-tui

# Create build directory
mkdir build && cd build

# Configure with CMake (FTXUI will be downloaded automatically)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)

# Run
./todo_tui
```

## Usage

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `n` | Add new todo |
| `e` | Edit selected todo |
| `d` | Delete selected todo |
| `t` | Toggle completion status |
| `f` | Filter (All ↔ Active ↔ Completed) |
| `s` | Search todos |
| `h` or `?` | Show help |
| `↑/↓` or `k/j` | Navigate list |
| `q` | Quit application |

### Adding a Todo

1. Press `n` to add a new todo
2. Enter the todo description
3. Select priority (1=Low, 2=Medium, 3=High)
4. Press Enter to save

### Filtering

Press `f` to cycle through filters:
- All todos
- Active todos
- Completed todos

## Project Structure

```
cpp-todo-tui/
├── CMakeLists.txt          # CMake build with FTXUI FetchContent
├── README.md               # This file
├── .gitignore              # Git ignore rules
├── src/
│   ├── main.cpp           # Application entry point
│   ├── app.cpp            # Main TUI logic using FTXUI
│   ├── app.hpp            # Application class declaration
│   ├── database.cpp       # SQLite database operations
│   ├── database.hpp       # Database class declaration
│   ├── models.cpp         # Todo model definitions
│   └── models.hpp         # Model class declarations
└── todo.db                # SQLite database (created on first run)
```

## Architecture

### FTXUI Components

The application uses FTXUI's **functional component model**:

```cpp
// Main UI structure
vbox({
    renderHeader(),      // Blue header bar
    separator(),         // Horizontal line
    renderTodoList(),    // Scrollable todo list
    separator(),
    renderStatusLine(),  // Statistics bar
    separator(),
    renderFooter()       // Keyboard shortcuts
}) | border;             // Outer border
```

### Key FTXUI Features Used:

- **Layout**: `vbox`, `hbox`, `flex`, `filler`
- **Styling**: `color`, `bgcolor`, `bold`, `dim`, `inverted`, `strikethrough`
- **Components**: `Renderer`, `CatchEvent`, `ScreenInteractive::Fullscreen()`
- **Decorators**: `border`, `separator`, `flex_grow`

### Models
- `Todo`: Represents a todo item with id, description, priority, status, and timestamps
- `Priority`: Enum for Low, Medium, High priorities
- `TodoStatus`: Enum for Active and Completed statuses

### Database
- SQLite-based persistence
- CRUD operations for todos
- Automatic schema creation on first run
- Prepared statements for security and performance

## Database Schema

```sql
CREATE TABLE IF NOT EXISTS todos (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    description TEXT NOT NULL,
    priority INTEGER NOT NULL DEFAULT 1,
    status INTEGER NOT NULL DEFAULT 0,
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    completed_at TEXT
);
```

## Dependencies

- **C++17** compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **FTXUI** - Modern functional TUI library (auto-downloaded via CMake)
- **SQLite3** - Embedded SQL database engine

## FTXUI vs ncurses

This application uses **FTXUI** instead of raw ncurses because:

| Feature | FTXUI | ncurses |
|---------|-------|---------|
| **Style** | Functional/React-like | Imperative/C-style |
| **Layout** | Easy vbox/hbox/grid | Manual positioning |
| **Colors** | Named colors, easy | Complex color pairs |
| **Widgets** | Built-in menus, inputs | Build from scratch |
| **UTF-8** | Full support | Limited |
| **Modern C++** | Yes, C++17+ | C-style APIs |
| **Dependencies** | None | ncurses library |

## License

MIT License - Feel free to use, modify, and distribute.

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## Troubleshooting

### Build Issues

**Error: FTXUI not found**
- FTXUI is automatically downloaded via CMake FetchContent
- Make sure you have internet connection during first build
- Or manually install FTXUI: `sudo apt-get install libftxui-dev`

**Error: SQLite3 not found**
```bash
sudo apt-get install libsqlite3-dev
```

### Runtime Issues

**Error: Terminal too small**
- Resize your terminal to at least 80x24 characters

**Error: Colors not showing**
- Make sure your terminal supports colors
- Try `export TERM=xterm-256color`

## Roadmap

- [ ] Due dates and reminders
- [ ] Categories/Tags support
- [ ] Export/Import functionality
- [ ] Cloud synchronization
- [ ] Dark/Light themes
- [ ] Multi-user support
- [ ] FTXUI animations for transitions

## Acknowledgments

- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) - Modern C++ TUI library by Arthur Sonzogni
- [SQLite](https://www.sqlite.org/) - Embedded SQL database engine

## Examples

### UI Preview

```
┌────────────────────────────────────────────────────────────────────┐
│                        TODO LIST                                   │
├────────────────────────────────────────────────────────────────────┤
│ [ ] HIGH  Buy groceries                                            │
│ [x] MEDIUM Finish project ✗                                        │
│ [ ] LOW   Call mom                                                 │
├────────────────────────────────────────────────────────────────────┤
│ Total: 3 | Active: 2 | Done: 1 | Filter: All                      │
├────────────────────────────────────────────────────────────────────┤
│ [n]New [e]Edit [d]Delete [t]Toggle [f]Filter [s]Search [h]Help [q]│
└────────────────────────────────────────────────────────────────────┘
```

---

Built with ❤️ using **FTXUI** and C++17