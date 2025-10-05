cmake_minimum_required(VERSION 3.16)
project(CppWorkspaceManager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Core Gui)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add source files
add_executable(cppm
    src/main.cpp
    src/Workspace.cpp
    src/WorkspaceManager.cpp
    src/MainWindow.cpp
)

# Include directories
target_include_directories(cppm PRIVATE include)

# Link Qt
target_link_libraries(cppm Qt5::Widgets Qt5::Core Qt5::Gui)
