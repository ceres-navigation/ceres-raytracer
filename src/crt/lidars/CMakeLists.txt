add_library(
    lidars
    lidar.hpp
    pencil_beam_lidar.hpp
)

set_target_properties(lidars PROPERTIES LINKER_LANGUAGE CXX)