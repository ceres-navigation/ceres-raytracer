cmake_minimum_required(VERSION 3.9)

project(ceres-raytracer CXX)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -mavx2 -mfma")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(BUILD_ARCH native CACHE STRING "Compile for this CPU architecture.")

add_subdirectory(lib)
include_directories(lib)

add_subdirectory(include)