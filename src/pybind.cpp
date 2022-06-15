#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "crt/transform.hpp"

#include <lodepng/lodepng.h>

#include "crt/cameras.hpp"
#include "crt/entity.hpp"
#include "crt/render.hpp"
#include "crt/path_trace.hpp"
#include "crt/lights.hpp"
#include "crt/passes.hpp"

#include "crt/static_scene.hpp"

#include "crt/obj_temp/obj.hpp"
#include "crt/materials/material.hpp"

namespace py = pybind11;

// Make this configurable at somepoint:
using Scalar = double;

using Vector3 = bvh::Vector3<Scalar>;

// Wrapper functions to handle type casting?  This seems weird to do it this way....
PinholeCamera<Scalar> create_pinhole(Scalar focal_length, py::list resolution_list, py::list sensor_size_list, bool z_positive) {
    Scalar resolution[2];
    Scalar sensor_size[2];

    resolution[0] = resolution_list[0].cast<Scalar>();
    resolution[1] = resolution_list[1].cast<Scalar>();

    sensor_size[0] = sensor_size_list[0].cast<Scalar>();
    sensor_size[1] = sensor_size_list[1].cast<Scalar>();

    return PinholeCamera<Scalar>(focal_length, resolution, sensor_size, z_positive);
}

SquareLight<Scalar> create_squarelight(Scalar intensity, py::list size_list){
    Scalar size[2];

    size[0] = size_list[0].cast<Scalar>();
    size[1] = size_list[1].cast<Scalar>();
    
    return SquareLight<Scalar>(intensity, size);
}

PointLight<Scalar> create_pointlight(Scalar intensity){
    return PointLight<Scalar>(intensity);
}

StaticEntity<Scalar> create_static_entity(std::string geometry_path, std::string geometry_type, bool smooth_shading, py::list color_list){
    Color color;
    color[0] = color_list[0].cast<Scalar>();
    color[1] = color_list[1].cast<Scalar>();
    color[2] = color_list[2].cast<Scalar>();
    
    return StaticEntity<Scalar>(geometry_path, geometry_type, smooth_shading, color);
}

Entity<Scalar>* create_entity(std::string geometry_path, std::string geometry_type, bool smooth_shading, py::list color_list){
    Color color;
    color[0] = color_list[0].cast<Scalar>();
    color[1] = color_list[1].cast<Scalar>();
    color[2] = color_list[2].cast<Scalar>();
    Entity<Scalar>* new_entity = new Entity<Scalar>(geometry_path, geometry_type, smooth_shading, color);
    return new_entity;
}

std::unique_ptr<CameraModel<Scalar>> copy_camera_unique(py::handle camera){
    std::unique_ptr<CameraModel<Scalar>> camera_copy;
    if (py::isinstance<PinholeCamera<Scalar>>(camera)){
        PinholeCamera<Scalar> camera_cast = camera.cast<PinholeCamera<Scalar>>();
        camera_copy = std::make_unique<PinholeCamera<Scalar>>(camera_cast);
    }
    else {
        // throw an exception
    }
    return camera_copy;
}

StaticScene<Scalar> create_static_scene(py::list static_entity_list) {
    // Convert py::list of entities to std::vector
    std::vector<Entity<Scalar>*> entities;
    uint32_t id = 1;
    for (auto static_entity_handle : static_entity_list) {
        StaticEntity<Scalar> static_entity = static_entity_handle.cast<StaticEntity<Scalar>>();

        //Create the new entities:
        Entity<Scalar>* new_entity = new Entity<Scalar>(static_entity.geometry_path, static_entity.geometry_type, static_entity.smooth_shading, static_entity.color);
        new_entity->set_scale(static_entity.scale);
        new_entity->set_position(static_entity.position);
        new_entity->set_rotation(static_entity.rotation);
        new_entity->set_id(id);
        id++;

        // Add new entity to vector:
        entities.emplace_back(new_entity);
    }

    return StaticScene(entities);
}

// Definition of the python wrapper module:
PYBIND11_MODULE(_crt, crt) {
    crt.doc() = "ceres ray tracer";

    py::class_<PinholeCamera<Scalar>>(crt, "PinholeCamera")
        .def(py::init(&create_pinhole))
        .def("set_position", [](PinholeCamera<Scalar> &self, py::array_t<Scalar> position){
            py::buffer_info buffer = position.request();
            Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
            auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
            self.set_position(position_vector3);
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
        .def("set_pose", [](PinholeCamera<Scalar> &self, py::array_t<Scalar> position, py::array_t<double, py::array::c_style | py::array::forcecast> rotation){
            // Get the position:
            py::buffer_info buffer_pos = position.request();
            Scalar *ptr_pos = static_cast<Scalar *>(buffer_pos.ptr);
            auto position_vector3 = Vector3(ptr_pos[0],ptr_pos[1],ptr_pos[2]);

            // Get the rotation:
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
            // Set the pose:
            self.set_pose(position_vector3, rotation_arr);
        });

    py::class_<PointLight<Scalar>>(crt, "PointLight")
        .def(py::init(&create_pointlight))
        .def("set_position", [](PointLight<Scalar> &self, py::array_t<Scalar> position){
            py::buffer_info buffer = position.request();
            Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
            auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
            self.set_position(position_vector3);
        })
        .def("set_rotation", [](PointLight<Scalar> &self, py::array_t<Scalar> rotation){
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
        .def("set_pose", [](PointLight<Scalar> &self, py::array_t<Scalar> position, py::array_t<Scalar> rotation){
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

    py::class_<SquareLight<Scalar>>(crt, "SquareLight")
        .def(py::init(&create_squarelight))
        .def("set_position", [](SquareLight<Scalar> &self, py::array_t<Scalar> position){
            py::buffer_info buffer = position.request();
            Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
            auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
            self.set_position(position_vector3);
        })
        .def("set_rotation", [](SquareLight<Scalar> &self, py::array_t<Scalar> rotation){
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
        .def("set_pose", [](SquareLight<Scalar> &self, py::array_t<Scalar> position, py::array_t<Scalar> rotation){
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

    py::class_<StaticEntity<Scalar>>(crt, "StaticEntity")
        .def(py::init(&create_static_entity))
        .def("set_scale",    [](StaticEntity<Scalar> &self, Scalar scale){ 
            self.set_scale(scale);
        })
        .def("set_position", [](StaticEntity<Scalar> &self, py::array_t<Scalar> position){
            py::buffer_info buffer = position.request();
            Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
            auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
            self.set_position(position_vector3);
        })
        .def("set_rotation", [](StaticEntity<Scalar> &self, py::array_t<Scalar> rotation){
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
        .def("set_pose", [](StaticEntity<Scalar> &self, py::array_t<Scalar> position, py::array_t<Scalar> rotation){
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

    py::class_<StaticScene<Scalar>>(crt, "StaticScene")
        .def(py::init(&create_static_scene))
        .def("render",    [](StaticScene<Scalar> &self, py::handle camera, py::list lights_list,
                             int min_samples, int max_samples, Scalar noise_threshold, int num_bounces){ 

            // Duplicate camera to obtain unique_ptr:
            auto camera_use = copy_camera_unique(camera);

            // Convert py::list of lights to std::vector
            std::vector<std::unique_ptr<Light<Scalar>>> lights;
            for (py::handle light : lights_list) { 
                if (py::isinstance<PointLight<Scalar>>(light)){
                    PointLight<Scalar> light_new = light.cast<PointLight<Scalar>>();
                    lights.push_back(std::make_unique<PointLight<Scalar>>(light_new));
                }
                else if (py::isinstance<SquareLight<Scalar>>(light)){
                    SquareLight<Scalar> light_new = light.cast<SquareLight<Scalar>>();
                    lights.push_back(std::make_unique<SquareLight<Scalar>>(light_new));
                }
            }

            // Call the render method:
            auto pixels = self.render(camera_use, lights, min_samples, max_samples, noise_threshold, num_bounces);

            // Format the output image:
            int width  = (size_t) floor(camera_use->get_resolutionX());
            int height = (size_t) floor(camera_use->get_resolutionY());
            auto result = py::array_t<uint8_t>({height,width,4});
            auto raw = result.mutable_data();
            for (int i = 0; i < height*width*4; i++) {
                raw[i] = pixels[i];
            }
            return result;
        })
        .def("intersection_pass", [](StaticScene<Scalar> &self, py::handle camera){
            // Duplicate camera to obtain unique_ptr:
            auto camera_use = copy_camera_unique(camera);

            // Call the intersection_pass method:
            auto intersections = self.intersection_pass(camera_use);

            // Format the output array:
            int width  = (size_t) floor(camera_use->get_resolutionX());
            int height = (size_t) floor(camera_use->get_resolutionY());
            auto result = py::array_t<Scalar>({height,width,3});
            auto raw = result.mutable_data();
            for (int i = 0; i < height*width*3; i++){
                raw[i] = intersections[i];
            }
            return result;

        })
        .def("instance_pass", [](StaticScene<Scalar> &self, py::handle camera){
            // Duplicate camera to obtain unique_ptr:
            auto camera_use = copy_camera_unique(camera);

            // Call the instance_pass method:
            auto instances = self.instance_pass(camera_use);

            // Format the output array:
            int width  = (size_t) floor(camera_use->get_resolutionX());
            int height = (size_t) floor(camera_use->get_resolutionY());
            auto result = py::array_t<uint32_t>({height,width});
            auto raw = result.mutable_data();
            for (int i = 0; i < height*width; i++){
                raw[i] = instances[i];
            }
            return result;
        })
        .def("normal_pass", [](StaticScene<Scalar> &self, py::handle camera){
            // Duplicate camera to obtain unique_ptr:
            auto camera_use = copy_camera_unique(camera);

            // Call the normal_pass method:
            auto normals = self.normal_pass(camera_use);

            // Format the output array:
            int width  = (size_t) floor(camera_use->get_resolutionX());
            int height = (size_t) floor(camera_use->get_resolutionY());
            auto result = py::array_t<Scalar>({height,width,3});
            auto raw = result.mutable_data();
            for (int i = 0; i < height*width*3; i++){
                raw[i] = normals[i];
            }
            return result;
        });

    crt.def("render", [](py::handle camera, py::list lights_list, py::list entity_list,
                         int min_samples, int max_samples, Scalar noise_threshold, int num_bounces){

        // Duplicate camera to obtain unique_ptr:
        auto camera_use = copy_camera_unique(camera);

        // Convert py::list of lights to std::vector
        std::vector<std::unique_ptr<Light<Scalar>>> lights;
        for (py::handle light : lights_list) { 
            if (py::isinstance<PointLight<Scalar>>(light)){
                PointLight<Scalar> light_new = light.cast<PointLight<Scalar>>();
                lights.push_back(std::make_unique<PointLight<Scalar>>(light_new));
            }
            else if (py::isinstance<SquareLight<Scalar>>(light)){
                SquareLight<Scalar> light_new = light.cast<SquareLight<Scalar>>();
                lights.push_back(std::make_unique<SquareLight<Scalar>>(light_new));
            }
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
        int width  = (size_t) floor(camera_use->get_resolutionX());
        int height = (size_t) floor(camera_use->get_resolutionY());
        auto result = py::array_t<uint8_t>({height,width,4});
        auto raw = result.mutable_data();
        for (int i = 0; i < height*width*4; i++) {
            raw[i] = pixels[i];
        }
        return result;
    });

    crt.def("intersection_pass", [](py::handle camera, py::list entity_list){
        // Duplicate camera to obtain unique_ptr:
        auto camera_use = copy_camera_unique(camera);

        // Convert py::list of entities to std::vector
        std::vector<Entity<Scalar>*> entities;
        for (auto entity_handle : entity_list) {
            Entity<Scalar>* entity = entity_handle.cast<Entity<Scalar>*>();
            entities.emplace_back(entity);
        }

        // Call the intersection tracing function:
        auto intersections = intersection_pass(camera_use, entities);

        // Format the output array:
        int width  = (size_t) floor(camera_use->get_resolutionX());
        int height = (size_t) floor(camera_use->get_resolutionY());
        auto result = py::array_t<Scalar>({height,width,3});
        auto raw = result.mutable_data();
        for (int i = 0; i < height*width*3; i++){
            raw[i] = intersections[i];
        }
        return result;
    });

    crt.def("instance_pass", [](py::handle camera, py::list entity_list){
        // Duplicate camera to obtain unique_ptr:
        auto camera_use = copy_camera_unique(camera);

        // Convert py::list of entities to std::vector
        uint32_t id = 1;
        std::vector<Entity<Scalar>*> entities;
        for (auto entity_handle : entity_list) {
            Entity<Scalar>* entity = entity_handle.cast<Entity<Scalar>*>();
            entity->set_id(id);
            entities.emplace_back(entity);
            id++;
        }

        // Call the intersection tracing function:
        auto instances = instance_pass(camera_use, entities);

        // Format the output array:
        int width  = (size_t) floor(camera_use->get_resolutionX());
        int height = (size_t) floor(camera_use->get_resolutionY());
        auto result = py::array_t<uint32_t>({height,width});
        auto raw = result.mutable_data();
        for (int i = 0; i < height*width; i++){
            raw[i] = instances[i];
        }
        return result;
    });

    crt.def("normal_pass", [](py::handle camera, py::list entity_list){
        // Duplicate camera to obtain unique_ptr:
        auto camera_use = copy_camera_unique(camera);

        // Convert py::list of entities to std::vector
        uint32_t id = 1;
        std::vector<Entity<Scalar>*> entities;
        for (auto entity_handle : entity_list) {
            Entity<Scalar>* entity = entity_handle.cast<Entity<Scalar>*>();
            entity->set_id(id);
            entities.emplace_back(entity);
            id++;
        }

        // Call the intersection tracing function:
        auto normals = normal_pass(camera_use, entities);

        // Format the output array:
        int width  = (size_t) floor(camera_use->get_resolutionX());
        int height = (size_t) floor(camera_use->get_resolutionY());
        auto result = py::array_t<Scalar>({height,width,3});
        auto raw = result.mutable_data();
        for (int i = 0; i < height*width*3; i++){
            raw[i] = normals[i];
        }

        return result;
    });
}