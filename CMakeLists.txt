cmake_minimum_required(VERSION 3.30)
project(noc C)

set(CMAKE_C_STANDARD 11)

add_executable(noc
        src/header/func_declaration.h
        src/header/data_structure.h
        src/routing/router_simulate.c
        main.c
        src/utils/memory_oper.c
        src/utils/io_utils.c
        src/traffic/traffic_pattern.c
        src/utils/initial_utils.c
        src/routing/routing_algorithm.c
        src/routing/routing_utils.c
        src/traffic/traffic_utils.c
)
#add_executable(new_target test-main.c)
#add_executable(new_target1 shuffle_traffic.c)
