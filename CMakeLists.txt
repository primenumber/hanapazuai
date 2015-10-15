cmake_minimum_required(VERSION 2.8)
project(hanapazu)

set(CMAKE_CXX_FLAGS "-std=c++14 -O3 -march=native -mtune=native -Wall -Wextra")
add_library(hanapazu board.cpp visualize.cpp play.cpp)
add_executable(ai main.cpp)
target_link_libraries(ai ncurses hanapazu)

