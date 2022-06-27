#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <lodepng/lodepng.h>

// CRT Includes:
#include "crt/cameras/camera.hpp"
#include "crt/cameras/simple_camera.hpp"

#include "crt/lidars/lidar.hpp"
#include "crt/lidars/simple_lidar.hpp"

#include "crt/lights/light.hpp"
#include "crt/lights/point_light.hpp"
#include "crt/lights/area_light.hpp"

#include "crt/rendering_body_fixed/body_fixed_entity.hpp"
#include "crt/rendering_body_fixed/body_fixed_group.hpp"

#include "crt/rendering_dynamic/entity.hpp"
#include "crt/rendering_dynamic/render.hpp"
#include "crt/rendering_dynamic/simulate_lidar.hpp"

#include "crt/passes.hpp"

namespace py = pybind11;

// Make this configurable at somepoint:
using Scalar = double;

using Vector3 = bvh::Vector3<Scalar>;

// Wrapper functions to handle type casting?  This seems weird to do it this way....
SimpleCamera<Scalar> create_simple_camera(Scalar focal_length, py::list resolution_list, py::list sensor_size_list, bool z_positive) {
    Scalar resolution[2];
    Scalar sensor_size[2];

    resolution[0] = resolution_list[0].cast<Scalar>();
    resolution[1] = resolution_list[1].cast<Scalar>();

    sensor_size[0] = sensor_size_list[0].cast<Scalar>();
    sensor_size[1] = sensor_size_list[1].cast<Scalar>();

    return SimpleCamera<Scalar>(focal_length, resolution, sensor_size, z_positive);
}

SimpleLidar<Scalar> create_simple_lidar(bool z_positive){
    return SimpleLidar<Scalar>(z_positive);
}

AreaLight<Scalar> create_AreaLight(Scalar intensity, py::list size_list){
    Scalar size[2];

    size[0] = size_list[0].cast<Scalar>();
    size[1] = size_list[1].cast<Scalar>();
    
    return AreaLight<Scalar>(intensity, size);
}

PointLight<Scalar> create_pointlight(Scalar intensity){
    return PointLight<Scalar>(intensity);
}

BodyFixedEntity<Scalar> create_body_fixed_entity(std::string geometry_path, std::string geometry_type, bool smooth_shading, py::list color_list){
    Color color;
    color[0] = color_list[0].cast<Scalar>();
    color[1] = color_list[1].cast<Scalar>();
    color[2] = color_list[2].cast<Scalar>();
    
    return BodyFixedEntity<Scalar>(geometry_path, geometry_type, smooth_shading, color);
}

Entity<Scalar>* create_entity(std::string geometry_path, std::string geometry_type, bool smooth_shading, py::list color_list){
    Color color;
    color[0] = color_list[0].cast<Scalar>();
    color[1] = color_list[1].cast<Scalar>();
    color[2] = color_list[2].cast<Scalar>();
    Entity<Scalar>* new_entity = new Entity<Scalar>(geometry_path, geometry_type, smooth_shading, color);
    return new_entity;
}

std::unique_ptr<Camera<Scalar>> get_camera_model(py::handle camera){
    std::unique_ptr<Camera<Scalar>> camera_ptr;
    if (py::isinstance<SimpleCamera<Scalar>>(camera)){
        SimpleCamera<Scalar> camera_cast = camera.cast<SimpleCamera<Scalar>>();
        camera_ptr = std::make_unique<SimpleCamera<Scalar>>(camera_cast);
    }
    else {
        // throw an exception
    }
    return camera_ptr;
}

std::unique_ptr<Lidar<Scalar>> get_lidar_model(py::handle lidar){
    std::unique_ptr<Lidar<Scalar>> lidar_ptr;
    if (py::isinstance<SimpleLidar<Scalar>>(lidar)){
        SimpleLidar<Scalar> lidar_cast = lidar.cast<SimpleLidar<Scalar>>();
        lidar_ptr = std::make_unique<SimpleLidar<Scalar>>(lidar_cast);
    }
    else {
        // throw and exception
    }
    return lidar_ptr;
}

BodyFixedGroup<Scalar> create_body_fixed_group(py::list body_fixed_entity_list) {
    // Convert py::list of entities to std::vector
    std::vector<Entity<Scalar>*> entities;
    uint32_t id = 1;
    for (auto body_fixed_entity_handle : body_fixed_entity_list) {
        BodyFixedEntity<Scalar> body_fixed_entity = body_fixed_entity_handle.cast<BodyFixedEntity<Scalar>>();

        //Create the new entities:
        Entity<Scalar>* new_entity = new Entity<Scalar>(body_fixed_entity.geometry_path, body_fixed_entity.geometry_type, body_fixed_entity.smooth_shading, body_fixed_entity.color);
        new_entity->set_scale(body_fixed_entity.scale);
        new_entity->set_position(body_fixed_entity.position);
        new_entity->set_rotation(body_fixed_entity.rotation);
        new_entity->set_id(id);
        id++;

        // Add new entity to vector:
        entities.emplace_back(new_entity);
    }

    return BodyFixedGroup(entities);
}

// Definition of the python wrapper module:
PYBIND11_MODULE(_crt, crt) {
    crt.doc() = "ceres ray tracer";

    py::class_<SimpleCamera<Scalar>>(crt, "SimpleCamera")
        .def(py::init(&create_simple_camera))
        .def("set_position", [](SimpleCamera<Scalar> &self, py::array_t<Scalar> position){
            py::buffer_info buffer = position.request();
            Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
            auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
            self.set_position(position_vector3);
        })
        .def("set_rotation", [](SimpleCamera<Scalar> &self, py::array_t<Scalar> rotation){
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
        .def("set_pose", [](SimpleCamera<Scalar> &self, py::array_t<Scalar> position, py::array_t<double, py::array::c_style | py::array::forcecast> rotation){
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

    py::class_<SimpleLidar<Scalar>>(crt, "SimpleLidar")
        .def(py::init(&create_simple_lidar))
        .def("set_position", [](SimpleLidar<Scalar> &self, py::array_t<Scalar> position){
            py::buffer_info buffer = position.request();
            Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
            auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
            self.set_position(position_vector3);
        })
        .def("set_rotation", [](SimpleLidar<Scalar> &self, py::array_t<Scalar> rotation){
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
        .def("set_pose", [](SimpleLidar<Scalar> &self, py::array_t<Scalar> position, py::array_t<Scalar> rotation){
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
        })
        .def("batch_set_pose", [](SimpleLidar<Scalar> &self, py::array_t<Scalar> batch_positions, 
                                                             py::array_t<Scalar> batch_rotations,
                                                             int number_of_poses){
            // Read the batch positions data:
            py::buffer_info buffer_batch_pos = batch_positions.request();
            Scalar *ptr_batch_pos = static_cast<Scalar *>(buffer_batch_pos.ptr);
            std::vector<bvh::Vector3<Scalar>> batch_positions_vector3;
            for (auto i = 0; i < number_of_poses; i++){
                auto position_vector3 = Vector3(ptr_batch_pos[0 + 3*i],
                                                ptr_batch_pos[1 + 3*i],
                                                ptr_batch_pos[2 + 3*i]);
                batch_positions_vector3.push_back(position_vector3);
            }

            // Read the batch rotations data:
            py::buffer_info buffer_batch_rot = batch_rotations.request();
            Scalar *ptr_batch_rot = static_cast<Scalar *>(buffer_batch_rot.ptr);

            Scalar batch_rotations_arr[3][3][100000] = {0};
            int idx = 0;
            for (auto i = 0; i < 3; i++){
                for (auto j = 0; j < 3; j++){
                    for (auto k = 0; k < 100000; k++){
                        batch_rotations_arr[i][j][k] = ptr_batch_rot[idx];
                        idx++;
                    }
                }
            }

            // Batch set the pose:
            self.batch_set_pose(batch_positions_vector3, batch_rotations_arr);
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

    py::class_<AreaLight<Scalar>>(crt, "AreaLight")
        .def(py::init(&create_AreaLight))
        .def("set_position", [](AreaLight<Scalar> &self, py::array_t<Scalar> position){
            py::buffer_info buffer = position.request();
            Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
            auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
            self.set_position(position_vector3);
        })
        .def("set_rotation", [](AreaLight<Scalar> &self, py::array_t<Scalar> rotation){
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
        .def("set_pose", [](AreaLight<Scalar> &self, py::array_t<Scalar> position, py::array_t<Scalar> rotation){
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

    py::class_<BodyFixedEntity<Scalar>>(crt, "BodyFixedEntity")
        .def(py::init(&create_body_fixed_entity))
        .def("set_scale",    [](BodyFixedEntity<Scalar> &self, Scalar scale){ 
            self.set_scale(scale);
        })
        .def("set_position", [](BodyFixedEntity<Scalar> &self, py::array_t<Scalar> position){
            py::buffer_info buffer = position.request();
            Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
            auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
            self.set_position(position_vector3);
        })
        .def("set_rotation", [](BodyFixedEntity<Scalar> &self, py::array_t<Scalar> rotation){
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
        .def("set_pose", [](BodyFixedEntity<Scalar> &self, py::array_t<Scalar> position, py::array_t<Scalar> rotation){
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

    py::class_<BodyFixedGroup<Scalar>>(crt, "BodyFixedGroup")
        .def(py::init(&create_body_fixed_group))
        .def("set_scale",    [](BodyFixedGroup<Scalar> &self, Scalar scale){ 
            self.set_scale(scale);
        })
        .def("set_position", [](BodyFixedGroup<Scalar> &self, py::array_t<Scalar> position){
            py::buffer_info buffer = position.request();
            Scalar *ptr = static_cast<Scalar *>(buffer.ptr);
            auto position_vector3 = Vector3(ptr[0],ptr[1],ptr[2]);
            self.set_position(position_vector3);
        })
        .def("set_rotation", [](BodyFixedGroup<Scalar> &self, py::array_t<Scalar> rotation){
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
        .def("set_pose", [](BodyFixedGroup<Scalar> &self, py::array_t<Scalar> position, py::array_t<Scalar> rotation){
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
        })
        .def("render", [](BodyFixedGroup<Scalar> &self, py::handle camera, py::list lights_list,
                          int min_samples, int max_samples, Scalar noise_threshold, int num_bounces){ 

            // Obtain the specific camera model:
            auto camera_ptr = get_camera_model(camera);

            // Convert py::list of lights to std::vector
            std::vector<std::unique_ptr<Light<Scalar>>> lights;
            for (py::handle light : lights_list) { 
                if (py::isinstance<PointLight<Scalar>>(light)){
                    PointLight<Scalar> light_new = light.cast<PointLight<Scalar>>();
                    lights.push_back(std::make_unique<PointLight<Scalar>>(light_new));
                }
                else if (py::isinstance<AreaLight<Scalar>>(light)){
                    AreaLight<Scalar> light_new = light.cast<AreaLight<Scalar>>();
                    lights.push_back(std::make_unique<AreaLight<Scalar>>(light_new));
                }
            }

            // Call the render method:
            auto pixels = self.render(camera_ptr, lights, min_samples, max_samples, noise_threshold, num_bounces);

            // Format the output image:
            int width  = (size_t) floor(camera_ptr->get_resolutionX());
            int height = (size_t) floor(camera_ptr->get_resolutionY());
            auto result = py::array_t<uint8_t>({height,width,4});
            auto raw = result.mutable_data();
            for (int i = 0; i < height*width*4; i++) {
                raw[i] = pixels[i];
            }
            return result;
        })
        .def("simulate_lidar", [](BodyFixedGroup<Scalar> &self, py::handle lidar, Scalar num_rays){
            // Obtain the specific lidar model:
            auto lidar_ptr = get_lidar_model(lidar);

            // Call the lidar method:
            auto distance = self.simulate_lidar(lidar_ptr, num_rays);

            return distance;
        })
        .def("batch_simulate_lidar", [](BodyFixedGroup<Scalar> &self, py::handle lidar, Scalar num_rays){
            // Obtain the specific lidar model:
            auto lidar_ptr = get_lidar_model(lidar);

            // Call the lidar method:
            auto distances = self.batch_simulate_lidar(lidar_ptr, num_rays);

            // Format the output array:
            int length = (size_t) distances.size();
            auto result = py::array_t<Scalar>({1,length});
            auto raw = result.mutable_data();
            int i = 0;
            for (Scalar dist : distances){
                raw[i] = dist;
                i++;
            }

            return result;
        })
        .def("intersection_pass", [](BodyFixedGroup<Scalar> &self, py::handle camera){
            // Obtain the specific camera model:
            auto camera_ptr = get_camera_model(camera);

            // Call the intersection_pass method:
            auto intersections = self.intersection_pass(camera_ptr);

            // Format the output array:
            int width  = (size_t) floor(camera_ptr->get_resolutionX());
            int height = (size_t) floor(camera_ptr->get_resolutionY());
            auto result = py::array_t<Scalar>({height,width,3});
            auto raw = result.mutable_data();
            for (int i = 0; i < height*width*3; i++){
                raw[i] = intersections[i];
            }
            return result;

        })
        .def("instance_pass", [](BodyFixedGroup<Scalar> &self, py::handle camera){
            // Obtain the specific camera model:
            auto camera_ptr = get_camera_model(camera);

            // Call the instance_pass method:
            auto instances = self.instance_pass(camera_ptr);

            // Format the output array:
            int width  = (size_t) floor(camera_ptr->get_resolutionX());
            int height = (size_t) floor(camera_ptr->get_resolutionY());
            auto result = py::array_t<uint32_t>({height,width});
            auto raw = result.mutable_data();
            for (int i = 0; i < height*width; i++){
                raw[i] = instances[i];
            }
            return result;
        })
        .def("normal_pass", [](BodyFixedGroup<Scalar> &self, py::handle camera){
            // Obtain the specific camera model:
            auto camera_ptr = get_camera_model(camera);

            // Call the normal_pass method:
            auto normals = self.normal_pass(camera_ptr);

            // Format the output array:
            int width  = (size_t) floor(camera_ptr->get_resolutionX());
            int height = (size_t) floor(camera_ptr->get_resolutionY());
            auto result = py::array_t<Scalar>({height,width,3});
            auto raw = result.mutable_data();
            for (int i = 0; i < height*width*3; i++){
                raw[i] = normals[i];
            }
            return result;
        });

    crt.def("render", [](py::handle camera, py::list lights_list, py::list entity_list,
                         int min_samples, int max_samples, Scalar noise_threshold, int num_bounces){

        // Obtain the specific camera model:
        auto camera_ptr = get_camera_model(camera);

        // Convert py::list of lights to std::vector
        std::vector<std::unique_ptr<Light<Scalar>>> lights;
        for (py::handle light : lights_list) { 
            if (py::isinstance<PointLight<Scalar>>(light)){
                PointLight<Scalar> light_new = light.cast<PointLight<Scalar>>();
                lights.push_back(std::make_unique<PointLight<Scalar>>(light_new));
            }
            else if (py::isinstance<AreaLight<Scalar>>(light)){
                AreaLight<Scalar> light_new = light.cast<AreaLight<Scalar>>();
                lights.push_back(std::make_unique<AreaLight<Scalar>>(light_new));
            }
        }

        // Convert py::list of entities to std::vector
        std::vector<Entity<Scalar>*> entities;
        for (auto entity_handle : entity_list) {
            Entity<Scalar>* entity = entity_handle.cast<Entity<Scalar>*>();
            entities.emplace_back(entity);
        }

        // Call the rendering function:
        auto pixels = render(camera_ptr, lights, entities,
                             min_samples, max_samples, noise_threshold, num_bounces);

        // Format the output image:
        int width  = (size_t) floor(camera_ptr->get_resolutionX());
        int height = (size_t) floor(camera_ptr->get_resolutionY());
        auto result = py::array_t<uint8_t>({height,width,4});
        auto raw = result.mutable_data();
        for (int i = 0; i < height*width*4; i++) {
            raw[i] = pixels[i];
        }
        return result;
    });

    crt.def("simulate_lidar", [](py::handle lidar, py::list entity_list, int num_rays){
        // OBtain the specific lidar model:
        auto lidar_ptr = get_lidar_model(lidar);

        // Convert py::list of entities to std::vector
        std::vector<Entity<Scalar>*> entities;
        for (auto entity_handle : entity_list) {
            Entity<Scalar>* entity = entity_handle.cast<Entity<Scalar>*>();
            entities.emplace_back(entity);
        }

        // Simulate the lidar:
        auto distance = simulate_lidar(lidar_ptr, entities, num_rays);

        return distance;
    });

    crt.def("intersection_pass", [](py::handle camera, py::list entity_list){
        // Obtain the specific camera model:
        auto camera_ptr = get_camera_model(camera);

        // Convert py::list of entities to std::vector
        std::vector<Entity<Scalar>*> entities;
        for (auto entity_handle : entity_list) {
            Entity<Scalar>* entity = entity_handle.cast<Entity<Scalar>*>();
            entities.emplace_back(entity);
        }

        // Call the intersection tracing function:
        auto intersections = intersection_pass(camera_ptr, entities);

        // Format the output array:
        int width  = (size_t) floor(camera_ptr->get_resolutionX());
        int height = (size_t) floor(camera_ptr->get_resolutionY());
        auto result = py::array_t<Scalar>({height,width,3});
        auto raw = result.mutable_data();
        for (int i = 0; i < height*width*3; i++){
            raw[i] = intersections[i];
        }
        return result;
    });

    crt.def("instance_pass", [](py::handle camera, py::list entity_list){
        // Obtain the specific camera model:
        auto camera_ptr = get_camera_model(camera);

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
        auto instances = instance_pass(camera_ptr, entities);

        // Format the output array:
        int width  = (size_t) floor(camera_ptr->get_resolutionX());
        int height = (size_t) floor(camera_ptr->get_resolutionY());
        auto result = py::array_t<uint32_t>({height,width});
        auto raw = result.mutable_data();
        for (int i = 0; i < height*width; i++){
            raw[i] = instances[i];
        }
        return result;
    });

    crt.def("normal_pass", [](py::handle camera, py::list entity_list){
        // Obtain the specific camera model:
        auto camera_ptr = get_camera_model(camera);

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
        auto normals = normal_pass(camera_ptr, entities);

        // Format the output array:
        int width  = (size_t) floor(camera_ptr->get_resolutionX());
        int height = (size_t) floor(camera_ptr->get_resolutionY());
        auto result = py::array_t<Scalar>({height,width,3});
        auto raw = result.mutable_data();
        for (int i = 0; i < height*width*3; i++){
            raw[i] = normals[i];
        }

        return result;
    });
}