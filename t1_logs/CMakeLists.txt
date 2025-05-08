cmake_minimum_required(VERSION 3.10)
project(ExpSort)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(ExpSort
    src/experimento.cpp
    src/mergesort_e.cpp
    src/quicksort_e.cpp
    src/generateSequences.cpp
)