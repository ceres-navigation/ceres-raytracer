cmake_minimum_required(VERSION 3.9)

project(_crt CXX)

set(CMAKE_CXX_STANDARD 17)

# Uncomment these lines to compile for debugging:
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS_DEBUG "-g")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -mtune=native -march=native -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fno-math-errno -fno-signed-zeros -fno-trapping-math -freciprocal-math -fno-rounding-math -fno-signaling-nans -fexcess-precision=fast")

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(lib)
add_subdirectory(src)