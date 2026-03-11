# C++ Todo TUI Application

A beautiful **Terminal User Interface (TUI)** Todo application built with modern C++17 and **ncurses**, featuring:

- 🎨 **Rich TUI** using ncurses with colors, borders, and interactive widgets
- 💾 **SQLite persistence** for reliable data storage
- ⚡ **Fast and responsive** with modern C++17 features
- ⌨️ **Keyboard shortcuts** for efficient navigation

## Features

- ✅ Add new todos with priority levels (Low, Medium, High)
- 📋 View all todos with filtering options
- ✏️ Edit existing todos
- 🗑️ Delete completed or unwanted todos
- 🎯 Filter by status (All, Active, Completed)
- 🔍 Search through todos
- 📊 View statistics and progress
- 💾 Automatic save to SQLite database

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libncurses5-dev libsqlite3-dev
```

### macOS
```bash
brew install cmake sqlite3 ncurses
```

### Windows (with MSYS2/MinGW)
```bash
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-sqlite3 mingw-w64-x86_64-ncurses
```

## Building

```bash
# Clone the repository
git clone https://github.com/1OOyan/cpp-todo-tui.git
cd cpp-todo-tui

# Create build directory
mkdir build && cd build

# Configure with CMake
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
| `f` | Filter todos (All/Active/Completed) |
| `s` | Search todos |
| `h` or `?` | Show help |
| `q` or `F10` | Quit application |
| `↑/↓` | Navigate list |
| `Page Up/Down` | Page through list |

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
├── CMakeLists.txt          # CMake build configuration
├── README.md               # This file
├── .gitignore              # Git ignore rules
├── src/
│   ├── main.cpp           # Application entry point
│   ├── app.cpp            # Main application logic and TUI
│   ├── app.hpp            # Application class declaration
│   ├── database.cpp       # SQLite database operations
│   ├── database.hpp       # Database class declaration
│   ├── models.cpp         # Todo model definitions
│   └── models.hpp         # Model class declarations
└── todo.db                # SQLite database (created on first run)
```

## Architecture

### Models
- `Todo`: Represents a todo item with id, description, priority, status, and timestamps
- `Priority`: Enum for Low, Medium, High priorities
- `TodoStatus`: Enum for Active and Completed statuses

### Database
- SQLite-based persistence
- CRUD operations for todos
- Automatic schema creation on first run
- Prepared statements for security and performance

### Application
- ncurses-based TUI framework
- Main window with todo list
- Dialogs for adding/editing todos
- Status bar with statistics
- Keyboard event handling

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
- **ncurses** - Terminal handling library
- **SQLite3** - Embedded SQL database engine

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

**Error: ncurses not found**
```bash
sudo apt-get install libncurses5-dev libncursesw5-dev
```

**Error: SQLite3 not found**
```bash
sudo apt-get install libsqlite3-dev
```

### Runtime Issues

**Error: Database locked**
- Close other applications using the database
- Delete `todo.db` and restart (data will be lost)

**Error: Terminal too small**
- Resize your terminal to at least 80x24 characters

## Roadmap

- [ ] Due dates and reminders
- [ ] Categories/Tags support
- [ ] Export/Import functionality
- [ ] Cloud synchronization
- [ ] Dark/Light themes
- [ ] Multi-user support

## Acknowledgments

- [ncurses](https://invisible-island.net/ncurses/) - Terminal handling library
- [SQLite](https://www.sqlite.org/) - Embedded SQL database engine

---

Built with ❤️ using C++17 and ncurses
