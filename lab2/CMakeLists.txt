add_executable(solver_lab2
    src/main.cpp
    src/solvers/local_search_solver.cpp
)

target_include_directories(solver_lab2
    PRIVATE
        src
        src/solvers
)

target_link_libraries(solver_lab2
    PRIVATE
        common_lib
        OpenMP::OpenMP_CXX
)
