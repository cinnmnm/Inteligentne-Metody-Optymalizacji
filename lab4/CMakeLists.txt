add_executable(solver_lab4
    src/main.cpp
    src/solvers/msls_solver.cpp
    src/solvers/ils_solver.cpp
    src/solvers/lns_solver.cpp
    ../../lab3/src/solvers/lab3_base_solver.cpp
)

target_include_directories(solver_lab4
    PRIVATE
        src
        src/solvers
)

target_link_libraries(solver_lab4
    PRIVATE
        common_lib
        OpenMP::OpenMP_CXX
)