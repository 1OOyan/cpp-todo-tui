cmake_minimum_required(VERSION 3.14)
project(todo_tui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(SQLite3 REQUIRED)
find_package(ftxui-screen REQUIRED)
find_package(ftxui-dom REQUIRED)
find_package(ftxui-component REQUIRED)

# Add executable
add_executable(todo_tui
    src/main.cpp
    src/app.cpp
    src/app.hpp
    src/database.cpp
    src/database.hpp
    src/models.cpp
    src/models.hpp
)

# Include directories
target_include_directories(todo_tui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries
target_link_libraries(todo_tui
    SQLite::SQLite3
    ftxui::screen
    ftxui::dom
    ftxui::component
)

# Install
target_install(TARGETS todo_tui DESTINATION bin)
