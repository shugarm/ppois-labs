cmake_minimum_required(VERSION 3.14)
project(lab1 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LAB1_BUILD_TESTS "Build unit tests (downloads GoogleTest)" ON)
option(LAB1_ENABLE_COVERAGE "Instrument the code for coverage measurement (GCC/Clang)" OFF)
option(LAB1_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

add_library(lab1_warnings INTERFACE)
if(MSVC)
  target_compile_options(lab1_warnings INTERFACE /W4 /utf-8)
  if(LAB1_WARNINGS_AS_ERRORS)
    target_compile_options(lab1_warnings INTERFACE /WX)
  endif()
else()
  target_compile_options(lab1_warnings INTERFACE -Wall -Wextra -Wpedantic)
  if(LAB1_WARNINGS_AS_ERRORS)
    target_compile_options(lab1_warnings INTERFACE -Werror)
  endif()
endif()

if(LAB1_ENABLE_COVERAGE AND NOT MSVC)
  add_compile_options(--coverage -O0 -g)
  add_link_options(--coverage)
endif()

# --- Пятнашки ---------------------------------------------------------------
add_library(fifteen_lib
  src/fifteen/FifteenPuzzle.cpp
  src/fifteen/FifteenMenu.cpp)
target_include_directories(fifteen_lib PUBLIC include)
target_link_libraries(fifteen_lib PRIVATE lab1_warnings)

add_executable(fifteen apps/fifteen_main.cpp)
target_link_libraries(fifteen PRIVATE fifteen_lib lab1_warnings)

# --- Машина Поста -----------------------------------------------------------
add_library(post_lib
  src/post/Tape.cpp
  src/post/Carriage.cpp
  src/post/Rule.cpp
  src/post/Program.cpp
  src/post/PostMachine.cpp
  src/post/PostMachineMenu.cpp
  src/post/PostMachineApp.cpp)
target_include_directories(post_lib PUBLIC include)
target_link_libraries(post_lib PRIVATE lab1_warnings)

add_executable(post_machine apps/post_main.cpp)
target_link_libraries(post_machine PRIVATE post_lib lab1_warnings)

# --- Установка (используется для сборки артефактов релиза) ------------------
install(TARGETS fifteen post_machine RUNTIME DESTINATION .)
install(DIRECTORY examples DESTINATION .)

# --- Модульные тесты --------------------------------------------------------
if(LAB1_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  add_executable(lab1_tests
    tests/fifteen_puzzle_tests.cpp
    tests/fifteen_menu_tests.cpp
    tests/tape_tests.cpp
    tests/carriage_tests.cpp
    tests/rule_tests.cpp
    tests/program_tests.cpp
    tests/post_machine_tests.cpp
    tests/post_machine_menu_tests.cpp
    tests/post_machine_app_tests.cpp)
  target_link_libraries(lab1_tests PRIVATE fifteen_lib post_lib GTest::gtest_main)

  include(GoogleTest)
  gtest_discover_tests(lab1_tests)
endif()
