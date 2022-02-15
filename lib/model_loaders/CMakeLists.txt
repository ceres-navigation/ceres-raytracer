add_library(
    model_loaders
    tiny_obj_loader.hpp
    happly.hpp
    tiny_gltf_loader.hpp
)

set_target_properties(model_loaders PROPERTIES LINKER_LANGUAGE CXX)