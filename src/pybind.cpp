#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "crt/rotations.hpp"
#include "crt/transform.hpp"

#include <lodepng/lodepng.h>

#include "crt/cameras.hpp"
#include "crt/entity.hpp"
#include "crt/ray_tracer.hpp"
#include "crt/lighting.hpp"

#include "crt/obj_temp/obj.hpp"
#include "crt/materials/material.hpp"

namespace py = pybind11;

// Make this configurable at somepoint:
using Scalar = double;

using Vector3 = bvh::Vector3<Scalar>;

// Wrapper functions to handle type casting?  This seems weird to do it this way....
PinholeCamera<Scalar> create_pinhole(Scalar focal_length, py::list resolution_list, py::list sensor_size_list) {
    Scalar resolution[2];
    Scalar sensor_size[2];

    resolution[0] = resolution_list[0].cast<Scalar>();
    resolution[1] = resolution_list[1].cast<Scalar>();

    sensor_size[0] = sensor_size_list[0].cast<Scalar>();
    sensor_size[1] = sensor_size_list[1].cast<Scalar>();

    return PinholeCamera<Scalar>(focal_length, resolution, sensor_size);
}

PointLight<Scalar> create_pointlight(Scalar intensity){
    return PointLight<Scalar>(intensity);
}

Entity<Scalar>* create_entity(std::string path_to_model, bool smooth_shading, py::list color_list){
    Color color;
    color[0] = color_list[0].cast<Scalar>();
    color[1] = color_list[1].cast<Scalar>();
    color[2] = color_list[2].cast<Scalar>();
    Entity<Scalar>* new_entity = new Entity<Scalar>(path_to_model, smooth_shading, color);
    return new_entity;
}

std::unique_ptr<CameraModel<Scalar>> copy_camera_unique(PinholeCamera<Scalar> camera){
    auto camera_copy = std::make_unique<PinholeCamera<Scalar>>(camera);
    return camera_copy;
}

// Definition of the python wrapper module:
PYBIND11_MODULE(_ceresrt, crt) {
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

    py::class_<Entity<Scalar>>(crt, "Entity")
        .def(py::init(&create_entity))
        .def("set_scale",    [](Entity<Scalar> &self, Scalar scale){ 
            self.set_scale(scale);
        })
        .def("set_position", [](Entity<Scalar> &self, py::array_t<Scalar> position){
                             py::buffer_info buffer = position.request();
                             Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
                             auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
                             self.set_position(position_vector3);
        })
        .def("get_position", [](Entity<Scalar> &self){
                             std::vector<Scalar> position_arr = {self.position[0],
                                                                 self.position[1],
                                                                 self.position[2]};
                             py::array position_out = py::cast(position_arr);
                             return position_out;
        })

        .def("set_rotation", [](Entity<Scalar> &self, py::array_t<Scalar> rotation){
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
        .def("get_rotation", [](Entity<Scalar> &self){
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
        .def("set_pose", [](Entity<Scalar> &self, py::array_t<Scalar> position, py::array_t<Scalar> rotation){
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
        crt.def("render", [](PinholeCamera<Scalar> &camera_in, py::list lights_list, py::list entity_list,
                          int min_samples, int max_samples, Scalar noise_threshold, int num_bounces){

        // Duplicate camera to obtain unique_ptr:
        auto camera_use = copy_camera_unique(camera_in);

        // Convert py::list of lights to std::vector
        std::vector<std::unique_ptr<Light<Scalar>>> lights;
        for (auto light_handle : lights_list) { 
            PointLight<Scalar> light = light_handle.cast<PointLight<Scalar>>();
            lights.push_back(std::make_unique<PointLight<Scalar>>(light));
        }

        // Convert py::list of entities to std::vector
        std::vector<Entity<Scalar>*> entities;
        for (auto entity_handle : entity_list) {
            Entity<Scalar>* entity = entity_handle.cast<Entity<Scalar>*>();
            entities.emplace_back(entity);
        }

        // Call the rendering function:
        auto pixels = render(camera_use, lights, entities,
                             min_samples, max_samples, noise_threshold, num_bounces);

        // Format the output image:
        int width  = (size_t) floor(camera_in.get_resolutionX());
        int height = (size_t) floor(camera_in.get_resolutionY());
        auto result = py::array_t<uint8_t>({height,width,4});
        auto raw = result.mutable_data();
        for (int i = 0; i < height*width*4; i++) {
            raw[i] = pixels[i];
        }
        return result;
    });
}