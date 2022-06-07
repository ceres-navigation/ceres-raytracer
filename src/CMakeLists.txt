add_executable(${PROJECT_NAME} main.cpp)

add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=16 )
add_definitions( -DMAGICKCORE_HDRI_ENABLE=1 )
find_package(ImageMagick 7 REQUIRED COMPONENTS Magick++ MagickCore )
include_directories(${ImageMagick_INCLUDE_DIRS})

find_package(pybind11 CONFIG)
pybind11_add_module(ceres_rt pybind.cpp)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ImageMagick_LIBRARIES} PUBLIC OpenMP::OpenMP_CXX model_loaders INIReader bvh crt obj_temp materials)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ImageMagick_LIBRARIES} PUBLIC model_loaders INIReader bvh crt obj_temp materials)
endif()

include_directories(${CMAKE_SOURCE_DIR}/lib)
add_subdirectory(crt)