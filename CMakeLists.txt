cmake_minimum_required(VERSION 3.21)

project(DEEPRED LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(SDL3 CONFIG REQUIRED)

add_executable(deepred
    src/simulator/main.cpp
    src/simulator/cpu.cpp
    src/simulator/memory.cpp
    src/simulator/worker_pipe.cpp
    src/simulator/include/cpu.h
    src/simulator/include/memory.h
    src/simulator/include/ppu.h
    src/simulator/include/worker_pipe.h
)

target_include_directories(deepred PRIVATE src/simulator/include)
target_compile_options(deepred PRIVATE -Wall -Wextra -Wshadow)
target_link_libraries(deepred PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets SDL3::SDL3)
