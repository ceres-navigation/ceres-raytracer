add_library(
    model_loaders
    happly.hpp
    obj.hpp
    tiny_gltf_loader.hpp
    tiny_obj_loader.hpp
)

set_target_properties(model_loaders PROPERTIES LINKER_LANGUAGE CXX)