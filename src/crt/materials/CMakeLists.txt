add_library(
    materials
    brdfs.hpp
    material.hpp
)

set_target_properties(materials PROPERTIES LINKER_LANGUAGE CXX)