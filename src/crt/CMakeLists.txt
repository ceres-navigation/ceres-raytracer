add_library(
    crt
    rotations.hpp
    transform.hpp
    cameras.cpp
    entity.hpp
    render.hpp
    scene.hpp
    lighting.hpp
)
target_include_directories(crt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(materials)
add_subdirectory(model_loaders)

set_target_properties(crt PROPERTIES LINKER_LANGUAGE CXX)