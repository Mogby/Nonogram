cmake_minimum_required(VERSION 3.15)

project(nonogram CXX)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(nonogram src/main.cpp)

target_include_directories(
  nonogram
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
