find_package(pybind11 CONFIG)
pybind11_add_module(_crt pybind.cpp)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE PUBLIC OpenMP::OpenMP_CXX bvh lodepng model_loaders crt cameras lights materials path_tracing rendering_body_fixed rendering_dynamic)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE PUBLIC bvh lodepng model_loaders crt cameras lights materials path_tracing rendering_body_fixed rendering_dynamic)
endif()

include_directories(${CMAKE_SOURCE_DIR}/lib)
add_subdirectory(crt)