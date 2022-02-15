add_subdirectory(bvh)
add_subdirectory(INIReader)
add_subdirectory(model_loaders)

target_include_directories(bvh PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(INIReader PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(model_loaders PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")