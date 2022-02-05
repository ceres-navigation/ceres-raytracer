add_library(
    render.hpp
    transform.hpp
    brdfs.hpp
    cameras.hpp
    rotations.hpp
    scene.hpp
)

set_target_properties(render.hpp PROPERTIES LINKER_LANGUAGE CXX)