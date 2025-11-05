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
    string_utils.cpp
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

FetchContent_Declare(
    fltk
    URL https://github.com/fltk/fltk/archive/refs/tags/release-1.3.8.tar.gz
)
set(FLTK_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_FLUID OFF CACHE BOOL "" FORCE)
set(FLTK_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(fltk)
FetchContent_GetProperties(fltk)

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

add_executable(integrator_gui
    integrator_gui.cpp
)

target_link_libraries(integrator_gui
    PRIVATE
        cursedarch
        fltk
)

target_include_directories(integrator_gui
    PRIVATE
        ${fltk_SOURCE_DIR}
        ${fltk_BINARY_DIR}
)
