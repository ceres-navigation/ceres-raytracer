add_library(
    lights
    light.hpp
    point_light.hpp
    area_light.hpp
)

set_target_properties(lights PROPERTIES LINKER_LANGUAGE CXX)