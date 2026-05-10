cmake_minimum_required(VERSION 4.2)
project(Chess)

set(CMAKE_CXX_STANDARD 20)

add_executable(Chess main.cpp
        terminal.h
        terminal.cpp
        gobang.h
        gobang.cpp)

add_executable(RestoreTerminal
        r_terminal.cpp
)