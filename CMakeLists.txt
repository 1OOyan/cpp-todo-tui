cmake_minimum_required(VERSION 3.10)
project(cpp-todo-tui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)

# Add executable
add_executable(todo_tui
    src/main.cpp
    src/app.cpp
    src/database.cpp
    src/models.cpp
)

# Link libraries
target_link_libraries(todo_tui PRIVATE
    SQLite::SQLite3
    ncurses
    Threads::Threads
)

# Include directories
target_include_directories(todo_tui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compiler flags
target_compile_options(todo_tui PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

# Install
install(TARGETS todo_tui DESTINATION bin)