cmake_minimum_required(VERSION 3.16)
project(cursedarch LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_library(cursedarch STATIC
    avl_tree.cpp
    hashtable.cpp
    DoublyLinkedList.cpp
    data_integrator.cpp
)

target_include_directories(cursedarch
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(cursedarch_tests
    KursAch.cpp
    tests/doubly_linked_list_test.cpp
    tests/hashtable_test.cpp
    tests/tree_test.cpp
    tests/integrator_test.cpp
    tests/integrator_use_cases_test.cpp
)

target_link_libraries(cursedarch_tests
    PRIVATE
        cursedarch
        GTest::gtest_main
)

add_test(NAME cursedarch_tests COMMAND cursedarch_tests)
