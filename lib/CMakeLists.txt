add_subdirectory(bvh)
add_subdirectory(model_loaders)
add_subdirectory(lodepng)

set(CMAKE_CXX_FLAGS "-fPIC") #???

target_include_directories(bvh PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(model_loaders PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(lodepng PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")