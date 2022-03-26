cmake_minimum_required(VERSION 3.14)

project("Functional Containers"
        VERSION 1.0.0
        LANGUAGES CXX)

option(FC_BUILD_TESTS "Enable LTL tests" OFF)
option(FC_GENERATE_DOCUMENTATION "Generate the functional containers documentation" OFF)
option(FC_CODE_COVERAGE "Generate Code Coverage" ON)

add_subdirectory(src/)

if(FC_BUILD_TESTS)
    message(STATUS "Build tests and benchmarks")
    add_subdirectory(Tests/)
endif()

if(FC_GENERATE_DOCUMENTATION)
    message(STATUS "Build documentation")
    add_subdirectory(doc/)
endif()
