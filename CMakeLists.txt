cmake_minimum_required(VERSION 3.9)

project(ceres-raytracer CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(include)

include_directories(include)

add_executable(render render.cpp)