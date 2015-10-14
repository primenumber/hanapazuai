cmake_minimum_required(VERSION 2.8)
project(hanapazu)

set(CMAKE_CXX_FLAGS "-std=c++14 -O3 -march=native -mtune=native -Wall -Wextra")
add_executable(ai main.cpp board.cpp visualize.cpp)
target_link_libraries(ai ncurses)

