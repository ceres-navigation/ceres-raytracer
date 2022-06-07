#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "crt/rotations.hpp"
#include "crt/transform.hpp"

#include "crt/cameras.hpp"
#include "crt/entity.hpp"
#include "crt/scene.hpp"
#include "crt/render.hpp"
#include "crt/lighting.hpp"

#include "crt/obj_temp/obj.hpp"
#include "crt/materials/material.hpp"

namespace py = pybind11;

// Make this configurable at somepoint:
using Scalar = double;

using Vector3 = bvh::Vector3<Scalar>;

// Wrapper functions to handle type casting?  This seems weird to do it this way....
PinholeCamera<Scalar> create_pinhole(Scalar focal_length, pybind11::list resolution_tup, pybind11::list sensor_size_tup) {
    Scalar resolution[2];
    Scalar sensor_size[2];

    resolution[0] = resolution_tup[0].cast<Scalar>();
    resolution[1] = resolution_tup[1].cast<Scalar>();

    sensor_size[0] = sensor_size_tup[0].cast<Scalar>();
    sensor_size[1] = sensor_size_tup[1].cast<Scalar>();

    return PinholeCamera<Scalar>(focal_length, resolution, sensor_size);
}

PointLight<Scalar> create_pointlight(Scalar intensity){
    return PointLight<Scalar>(intensity);
}

// Definition of the python wrapper module:
PYBIND11_MODULE(ceres_rt, crt) {
    crt.doc() = "ceres ray tracer";

    py::class_<PinholeCamera<Scalar>>(crt, "PinholeCamera")
        .def(py::init(&create_pinhole))
        .def("set_position", [](PinholeCamera<Scalar> &self, py::array_t<Scalar> position){
                             py::buffer_info buffer = position.request();
                             Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
                             auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
                             self.set_position(position_vector3);
        })
        .def("get_position", [](PinholeCamera<Scalar> &self){
                             std::vector<Scalar> position_arr = {self.position[0],
                                                                 self.position[1],
                                                                 self.position[2]};
                             py::array position_out = py::cast(position_arr);
                             return position_out;
        })
        .def("set_rotation", [](PinholeCamera<Scalar> &self, py::array_t<Scalar> rotation){
                             py::buffer_info buffer = rotation.request();
                             Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
                             Scalar rotation_arr[3][3];
                             int idx = 0;
                             for (auto i = 0; i < 3; i++){
                                for (auto j = 0; j < 3; j++){
                                    rotation_arr[i][j] = ptr[idx];
                                    idx++;
                                }
                            }
                             self.set_rotation(rotation_arr);
        })
        .def("get_rotation", [](PinholeCamera<Scalar> &self){
                            auto rotation_arr = std::vector<std::vector<Scalar>>();
                            for (auto i = 0; i < 3; i++) {
                                rotation_arr.push_back(std::vector<Scalar>(3));
                            }
                            for (auto i = 0; i < 3; i++){
                                for (auto j = 0; j < 3; j++){
                                    rotation_arr[i][j] = self.rotation[i][j];
                                }
                            }
                             py::array rotation_out = py::cast(rotation_arr);
                             return rotation_out;
        })
        .def("set_pose", [](PinholeCamera<Scalar> &self, py::array_t<Scalar> position, py::array_t<Scalar> rotation){
                         // Set the position:
                         py::buffer_info buffer_pos = position.request();
                         Scalar *ptr_pos = static_cast<Scalar *>(buffer_pos.ptr);
                         auto position_vector3 = Vector3(ptr_pos[0],ptr_pos[1],ptr_pos[2]);
                         self.set_position(position_vector3);

                        // Set the rotation:
                         py::buffer_info buffer_rot = rotation.request();
                         Scalar *ptr_rot = static_cast<Scalar *>(buffer_rot.ptr);
                         Scalar rotation_arr[3][3];
                         int idx = 0;
                         for (auto i = 0; i < 3; i++){
                            for (auto j = 0; j < 3; j++){
                                rotation_arr[i][j] = ptr_rot[idx];
                                idx++;
                            }
                         }
                         self.set_rotation(rotation_arr);
        });

    py::class_<PointLight<Scalar>>(crt, "PointLight")
        .def(py::init(&create_pointlight))
        .def("set_position", [](PointLight<Scalar> &self, py::array_t<Scalar> position){
                             py::buffer_info buffer = position.request();
                             Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
                             auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
                             self.set_position(position_vector3);
        })
        .def("get_position", [](PointLight<Scalar> &self){
                             std::vector<Scalar> position_arr = {self.position[0],
                                                                 self.position[1],
                                                                 self.position[2]};
                             py::array position_out = py::cast(position_arr);
                             return position_out;
        });
}