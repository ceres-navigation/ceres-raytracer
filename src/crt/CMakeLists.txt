add_library(
    crt
    entity.hpp
    render.hpp
    scene.hpp
)
target_include_directories(crt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(tools)
add_subdirectory(materials)
add_subdirectory(model_loaders)
add_subdirectory(cameras)
add_subdirectory(lighting)

set_target_properties(crt PROPERTIES LINKER_LANGUAGE CXX)