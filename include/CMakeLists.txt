FILE(GLOB BVHSources bvh/*.hpp)

add_library(
    bvh
    ${BVHSources}
)

set_target_properties(bvh PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(bvh PUBLIC '${CMAKE_CURRENT_SOURCE_DIR}/bvh')