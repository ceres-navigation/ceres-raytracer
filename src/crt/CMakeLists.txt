add_library(
    crt
    transform.hpp
    render.hpp
    passes.hpp
    body_fixed.hpp
)
target_include_directories(crt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(materials)
add_subdirectory(models)
add_subdirectory(path_tracing)
add_subdirectory(cameras)
add_subdirectory(lights)

set_target_properties(crt PROPERTIES LINKER_LANGUAGE CXX)