add_library(
    crt
    rotations.hpp
    transform.hpp
    cameras.hpp
    entity.hpp
    render.hpp
    path_trace.hpp
    lighting.hpp
    scene_config.hpp
)
target_include_directories(crt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(materials)
add_subdirectory(obj_temp)

set_target_properties(crt PROPERTIES LINKER_LANGUAGE CXX)