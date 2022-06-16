add_library(
    materials
    brdfs.hpp
    btdfs.hpp
    material.hpp
    uvmaps.hpp
)

set_target_properties(materials PROPERTIES LINKER_LANGUAGE CXX)