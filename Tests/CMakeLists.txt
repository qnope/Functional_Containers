# ---- Tests ----

include(FetchContent)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(googletest
        GIT_REPOSITORY "https://github.com/google/googletest.git"
        GIT_TAG release-1.11.0#e2239ee6043f73722e7aa812a459f54a28552929
        UPDATE_DISCONNECTED YES)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(fc_map_test test_map.cpp)
add_executable(fc_zip_test test_zip.cpp)
add_executable(fc_algos_test test_algos.cpp)
add_executable(fc_filter_test test_filter.cpp)
add_executable(fc_optional_test test_optional.cpp)

target_link_libraries(fc_map_test PRIVATE FC::FC gtest_main)
target_link_libraries(fc_zip_test PRIVATE FC::FC gtest_main)
target_link_libraries(fc_algos_test PRIVATE FC::FC gtest_main)
target_link_libraries(fc_filter_test PRIVATE FC::FC gtest_main)
target_link_libraries(fc_optional_test PRIVATE FC::FC gtest_main)

add_test(NAME fc_map_test COMMAND fc_map_test)
add_test(NAME fc_zip_test COMMAND fc_algos_test)
add_test(NAME fc_algos_test COMMAND fc_algos_test)
add_test(NAME fc_filter_test COMMAND fc_filter_test)
add_test(NAME fc_optional_test COMMAND fc_optional_test)
