add_library(
    rendering_dynamic
    render.hpp
    entity.hpp
    simulate_lidar.hpp
)

set_target_properties(rendering_dynamic PROPERTIES LINKER_LANGUAGE CXX)