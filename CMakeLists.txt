cmake_minimum_required(VERSION 3.20)
project(shamigo C)

set(CMAKE_C_STANDARD 11)

option(USE_SANITIZER "Enable AddressSanitizer" OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic")

if(USE_SANITIZER)
    message(STATUS "Sanitizer ENABLED")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
endif()

include_directories(include)

file(GLOB SOURCES src/*.c)

add_executable(shamigo ${SOURCES})

