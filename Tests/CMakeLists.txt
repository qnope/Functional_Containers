# ---- Tests ----

include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(googletest
        GIT_REPOSITORY "https://github.com/google/googletest.git"
        GIT_TAG release-1.11.0#e2239ee6043f73722e7aa812a459f54a28552929
        UPDATE_DISCONNECTED YES)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(fc_algos_test test_algos.cpp)
add_executable(fc_optional_test test_optional.cpp)

target_link_libraries(fc_algos_test PRIVATE FC::FC gtest_main)
target_link_libraries(fc_optional_test PRIVATE FC::FC gtest_main)

add_test(NAME fc_optional_test COMMAND fc_optional_test)
add_test(NAME fc_algos_test COMMAND fc_algos_test)
