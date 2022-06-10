find_package(pybind11 CONFIG)
pybind11_add_module(_crt pybind.cpp)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE PUBLIC OpenMP::OpenMP_CXX model_loaders bvh lodepng crt obj_temp materials)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE PUBLIC model_loaders bvh lodepng crt obj_temp materials)
endif()

include_directories(${CMAKE_SOURCE_DIR}/lib)
add_subdirectory(crt)