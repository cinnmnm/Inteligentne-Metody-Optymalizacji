add_library(common_lib STATIC
    src/solution_checker.cpp
    src/io/instance_reader.cpp
    src/io/result_printer.cpp
)

target_include_directories(common_lib
    PUBLIC
        include
)

target_link_libraries(common_lib
    PUBLIC
        OpenMP::OpenMP_CXX
)
