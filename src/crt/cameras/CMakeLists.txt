add_library(
    cameras
    camera.hpp
    simple_camera.hpp
)

set_target_properties(cameras PROPERTIES LINKER_LANGUAGE CXX)