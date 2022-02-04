FILE(GLOB BVHSources bvh/*.hpp)

add_library(
    bvh
    ${BVHSources}
    obj.hpp
    toml.h
)

set_target_properties(bvh PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(bvh PUBLIC '${CMAKE_CURRENT_SOURCE_DIR}/bvh')