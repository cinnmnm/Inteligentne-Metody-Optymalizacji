cmake_minimum_required(VERSION 3.16)
project(tsp_profit_solver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenMP REQUIRED)

add_subdirectory(common)
add_subdirectory(lab1)
add_subdirectory(lab2)
