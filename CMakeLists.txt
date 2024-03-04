cmake_minimum_required(VERSION 3.21)
project(untitled)

set(CMAKE_CXX_STANDARD 14)


add_executable(untitled main.cpp)

target_link_libraries(untitled -lfreeglut -lopengl32 -lglu32 -lglew32)
