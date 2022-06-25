add_library(
    crt
    transform.hpp
    do_render.hpp
)
target_include_directories(crt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(cameras)
add_subdirectory(lights)
add_subdirectory(materials)
add_subdirectory(path_tracing)
add_subdirectory(rendering_body_fixed)
add_subdirectory(rendering_dynamic)

set_target_properties(crt PROPERTIES LINKER_LANGUAGE CXX)