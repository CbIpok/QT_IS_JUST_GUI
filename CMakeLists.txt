cmake_minimum_required(VERSION 3.16)

project(KursAch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Prefer static libstdc++/libgcc on Windows MinGW to ease runtime
if (MINGW)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

include(FetchContent)
set(BUILD_GMOCK OFF CACHE BOOL "Disable gmock" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "Disable gtest install" FORCE)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Project sources (library code under test)
set(KURSACH_SOURCES
  hashtable.cpp
  DoublyLinkedList.cpp
  avl_tree.cpp
)

add_library(kursach_lib ${KURSACH_SOURCES})
target_include_directories(kursach_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Tests
add_executable(kursach_tests
  KursAch.cpp
  tests/hashtable_test.cpp
  tests/doubly_linked_list_test.cpp
  tests/tree_test.cpp
)
target_link_libraries(kursach_tests PRIVATE kursach_lib GTest::gtest)
target_compile_definitions(kursach_tests PRIVATE GTEST_LANG_CXX11=1)

add_test(NAME all_tests COMMAND kursach_tests)

