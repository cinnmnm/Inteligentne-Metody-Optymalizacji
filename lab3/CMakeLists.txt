add_executable(solver_lab3
    src/main.cpp
    src/solvers/lab3_base_solver.cpp
    src/solvers/candidate_local_search_solver.cpp
    src/solvers/list_memory_local_search_solver.cpp
    src/solvers/hybrid_local_search_solver.cpp
    src/solvers/lazy_priority_queue_solver.cpp
)

target_include_directories(solver_lab3
    PRIVATE
        src
        src/solvers
)

target_link_libraries(solver_lab3
    PRIVATE
        common_lib
        OpenMP::OpenMP_CXX
)
