add_library(
    crt
    transform.hpp
    do_render.hpp
    rigid_body.hpp
    do_lidar.hpp
)
target_include_directories(crt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(cameras)
add_subdirectory(lidars)
add_subdirectory(lights)
add_subdirectory(materials)
add_subdirectory(path_tracing)
add_subdirectory(rendering_body_fixed)
add_subdirectory(rendering_dynamic)
add_subdirectory(sensors)

set_target_properties(crt PROPERTIES LINKER_LANGUAGE CXX)