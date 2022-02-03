add_executable(render render.cpp)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(render PUBLIC OpenMP::OpenMP_CXX)
endif()