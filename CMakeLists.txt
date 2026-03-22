cmake_minimum_required(VERSION 3.16)
project(tsp_profit_solver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenMP REQUIRED)

add_executable(solver
    src/main.cpp
    src/io/instance_reader.cpp
    src/io/result_printer.cpp
    src/solution_checker.cpp
    src/solvers/random_solver.cpp
    src/solvers/constructive_heuristic_solver.cpp
    src/solvers/nearest_neighbor_solver.cpp
    src/solvers/greedy_cycle_solver.cpp
    src/solvers/regret_solver.cpp
)

target_link_libraries(solver PRIVATE OpenMP::OpenMP_CXX)

target_include_directories(solver PRIVATE src)
