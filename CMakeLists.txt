cmake_minimum_required(VERSION 3.30)
project(noc C)

set(CMAKE_C_STANDARD 11)

add_executable(noc
        func_declaration.h
        data_structure.h
        router.c
        main.c
        memory_oper.c
        io_utils.c
        traffic_oper.c
        initial_utils.c
)
add_executable(new_target test-main.c)
