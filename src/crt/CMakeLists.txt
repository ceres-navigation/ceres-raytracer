add_library(
    crt
    transform.hpp
    cameras.hpp
    entity.hpp
    render.hpp
    passes.hpp
    path_trace.hpp
    static_scene.hpp
    lights.hpp
)
target_include_directories(crt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(materials)
add_subdirectory(obj_temp)

set_target_properties(crt PROPERTIES LINKER_LANGUAGE CXX)