add_library(
    lib
    INIReader.h
    happly.hpp
)

add_subdirectory(bvh)

set_target_properties(lib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(bvh PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")