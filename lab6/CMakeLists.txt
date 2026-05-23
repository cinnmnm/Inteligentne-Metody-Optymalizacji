add_executable(solver_lab6
    src/main.cpp
    src/solvers/greedy_heuristic_solver.cpp
    src/solvers/hae_solver.cpp
    ../lab3/src/solvers/lab3_base_solver.cpp
    ../lab3/src/solvers/hybrid_local_search_solver.cpp
    ../lab4/src/solvers/msls_solver.cpp
    ../lab4/src/solvers/ils_solver.cpp
    ../lab4/src/solvers/lns_solver.cpp
)

target_include_directories(solver_lab6
    PRIVATE
        src
        src/solvers
        ../lab3/src/solvers
        ../lab4/src/solvers
)

target_link_libraries(solver_lab6
    PRIVATE
        common_lib
        OpenMP::OpenMP_CXX
)