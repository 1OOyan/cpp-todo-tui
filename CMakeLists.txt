cmake_minimum_required(VERSION 3.14)
project(cpp-todo-tui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(SQLite3 REQUIRED)

# Try to find FTXUI first (if already installed)
find_package(ftxui-screen QUIET)

# If not found, fetch FTXUI from GitHub
if(NOT ftxui-screen_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v5.0.0
    )
    # Don't build examples/tests, just the libraries
    set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(FTXUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(ftxui)
    
    # Add FTXUI include directory when fetched
    if(FTXUI_SOURCE_DIR)
        include_directories(${FTXUI_SOURCE_DIR}/include)
    endif()
endif()

# Add executable
add_executable(todo_tui
    src/main.cpp
    src/app.cpp
    src/database.cpp
    src/models.cpp
)

# Include directories
target_include_directories(todo_tui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries - use imported targets when available, fallback to library names
if(ftxui-screen_FOUND)
    target_link_libraries(todo_tui PRIVATE
        SQLite::SQLite3
        ftxui::screen
        ftxui::dom
        ftxui::component
    )
else()
    target_link_libraries(todo_tui PRIVATE
        SQLite::SQLite3
        ftxui::screen
        ftxui::dom
        ftxui::component
    )
endif()

# Compiler flags
target_compile_options(todo_tui PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

# Install
install(TARGETS todo_tui DESTINATION bin)