add_library(
    lidars
    lidar.hpp
    simple_lidar.hpp
)

set_target_properties(lidars PROPERTIES LINKER_LANGUAGE CXX)