add_executable(solver_lab1
    src/main.cpp
    src/solvers/random_solver.cpp
    src/solvers/constructive_heuristic_solver.cpp
    src/solvers/nearest_neighbor_solver.cpp
    src/solvers/greedy_cycle_solver.cpp
    src/solvers/regret_solver.cpp
)

target_include_directories(solver_lab1
    PRIVATE
        src/solvers
)

target_link_libraries(solver_lab1
    PRIVATE
        common_lib
        OpenMP::OpenMP_CXX
)
