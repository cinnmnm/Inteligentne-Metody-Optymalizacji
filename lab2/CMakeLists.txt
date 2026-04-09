add_executable(solver_lab2
    src/main.cpp
)

target_link_libraries(solver_lab2
    PRIVATE
        common_lib
        OpenMP::OpenMP_CXX
)
