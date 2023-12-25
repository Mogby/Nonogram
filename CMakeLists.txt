cmake_minimum_required(VERSION 3.15)

project(nonogram CXX)
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost COMPONENTS program_options REQUIRED)

add_executable(nonogram src/main.cpp src/nonogram.cpp)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  nonogram
  PRIVATE
  Boost::program_options
)
