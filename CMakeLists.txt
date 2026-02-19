cmake_minimum_required(VERSION 3.24)

project(Rocket CXX)

include(cmake/RocketUtils.cmake)

add_subdirectory(deps EXCLUDE_FROM_ALL)

enable_testing()
add_subdirectory(platform)
