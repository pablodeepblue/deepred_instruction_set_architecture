cmake_minimum_required(VERSION 3.30)
project(mem_sim_)

set(CMAKE_CXX_STANDARD 20)

# Qt6 for Linux
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# SDL3 for Linux
find_package(SDL3 REQUIRED)

# Enable automatic moc, uic, and rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories for your headers
include_directories(include)
include_directories(cpp)
include_directories(load)
include_directories(include/SDL3)

add_library(SDL3 SHARED IMPORTED)
set_target_properties(SDL3 PROPERTIES
        IMPORTED_LOCATION "/usr/lib/libSDL3.so"
        IMPORTED_IMPLIB "/usr/lib/libSDL3.a"
)

add_executable(mem_sim_
        cpp/main.cpp
        cpp/memory.cpp
        cpp/cpu.cpp
        include/cpu.h
        cpp/worker_pipe.cpp
        include/worker_pipe.h
        cpp/ui.cpp
        include/ui.h
)

# Link libraries for Qt6 and SDL3
target_link_libraries(mem_sim_
        Qt::Core
        Qt::Gui
        Qt::Widgets
        SDL3
)
