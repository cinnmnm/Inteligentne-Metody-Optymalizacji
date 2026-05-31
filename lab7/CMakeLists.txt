add_executable(solver_lab7
    src/main.cpp
    src/solvers/island_model_hae_solver.cpp
    src/solvers/sa_lns_solver.cpp
    src/solvers/unified_hybrid_solver.cpp
    src/solvers/parallel_uhma_solver.cpp
    ../lab6/src/solvers/hae_solver.cpp
    ../lab3/src/solvers/lab3_base_solver.cpp
    ../lab3/src/solvers/hybrid_local_search_solver.cpp
    ../lab4/src/solvers/msls_solver.cpp
    ../lab4/src/solvers/ils_solver.cpp
    ../lab4/src/solvers/lns_solver.cpp
)

target_include_directories(solver_lab7
    PRIVATE
        src
        src/solvers
        ../lab6/src/solvers
        ../lab3/src/solvers
        ../lab4/src/solvers
)

target_link_libraries(solver_lab7
    PRIVATE
        common_lib
        OpenMP::OpenMP_CXX
)
