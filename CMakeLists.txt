cmake_minimum_required (VERSION 3.18)
project (thunderstorm)

set(CMAKE_BUILD_TYPE "RelWithDebInfo")
# set(CMAKE_BUILD_TYPE Debug)
find_package(fmt REQUIRED)

find_package(benchmark REQUIRED)
add_executable(thunderstorm thunderstorm.cpp)
target_link_libraries(thunderstorm PUBLIC fmt::fmt-header-only benchmark::benchmark)
