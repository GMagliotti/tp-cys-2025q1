cmake_minimum_required(VERSION 3.20)
project(tp-cys-2025q1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -fsanitize=address")

add_executable(tp-cys-2025q1
        main.c
        bmp.c
        file_utils.c
        sss.c
        sss_algos.c
        sss_helpers.c
        permutation_table.c)