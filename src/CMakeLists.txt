add_library(FC INTERFACE)

add_subdirectory(fc)

set(CMAKE_CXX_EXTENSIONS false)

#useful for IDEs
target_include_directories(FC INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

string(TOLOWER "${CMAKE_BUILD_TYPE}" FC_BUILD_TYPE)

if(MSVC)
    target_compile_options(FC INTERFACE /Zc:preprocessor /Zc:__cplusplus /permissive- /bigobj)
elseif(FC_CODE_COVERAGE AND FC_BUILD_TYPE MATCHES debug)
    message(STATUS "Build with code coverage")
    target_compile_options(FC INTERFACE -fprofile-arcs -ftest-coverage)
    target_link_options(FC INTERFACE -fprofile-arcs -ftest-coverage)
endif()

if(NOT MSVC)
    include(CheckCXXCompilerFlag)

    check_cxx_compiler_flag(-fcoroutines has_fcoroutines)
    check_cxx_compiler_flag(-fconcepts has_fconcepts)

    if(has_fcoroutines)
        target_compile_options(FC INTERFACE $<$<COMPILE_FEATURES:cxx_std_20>:-fcoroutines>)
    endif()

    if(has_fconcepts)
        target_compile_options(FC INTERFACE $<$<COMPILE_FEATURES:cxx_std_20>:-fconcepts>)
    endif()
endif()

target_compile_features(FC INTERFACE cxx_std_20)

add_library(FC::FC ALIAS FC)
