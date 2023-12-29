cmake_minimum_required(VERSION 3.15)

project(nonogram CXX)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(GTest)

add_executable(nonogram src/main.cpp src/nonogram.cpp)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  nonogram
  PRIVATE
  Boost::program_options
)

enable_testing()
add_executable(run_tests src/test.cpp src/nonogram.cpp)
target_link_libraries(
  run_tests
  GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(run_tests)
