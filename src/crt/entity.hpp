#ifndef __ENTITY_H
#define __ENTITY_H

#include <filesystem>
#include <memory>
#include <vector>
#include <random>

#include "bvh/bvh.hpp"
#include "bvh/triangle.hpp"
#include "bvh/vector.hpp"

#include "model_loaders/happly.hpp"
#include "model_loaders/tiny_obj_loader.hpp"

#include "rotations.hpp"
#include "transform.hpp"

#include "obj_temp/obj.hpp"
#include "materials/material.hpp"

template <typename Scalar>
class Entity {
    public:
        bvh::Vector3<Scalar> position;
        Scalar rotation[3][3];
        Scalar scale;
        uint32_t id;

        std::vector<bvh::Triangle<Scalar>> triangles;
        std::vector<std::shared_ptr<Material<Scalar>>> materials;
        std::shared_ptr<UVMap<size_t>> material_map;
        bool smooth_shading;

        Entity(std::string path_to_model, bool smooth_shading, Color color){
            // Load the mesh geometry:
            std::vector<bvh::Triangle<Scalar>> new_triangles;
            std::string extension = std::filesystem::path(path_to_model).extension();
            std::transform(extension.begin(), extension.end(), extension.begin(), static_cast<int(*)(int)>(std::tolower));
            if (extension.compare(".obj") == 0) {
                new_triangles = obj::load_from_file<Scalar>(path_to_model);
            } 
            else { 
                std::cout << "file type of " << extension << " is not a valid.  ceres-rt only supports .obj/.OBJ\n";
            }
            std::cout << new_triangles.size() << " triangles loaded from " << path_to_model << "\n";

            // Set current entity as the parent object for all input triangles:
            for (auto &tri : new_triangles) {
                tri.set_parent(this);
                this -> triangles.push_back(tri);
            }

            this->smooth_shading = smooth_shading;

            //TODO: REMOVE ALL OF THE HARDCODED STUFF HERE:
            this->materials.emplace_back(new ColoredLambertianMaterial<Scalar>(color));
            this->material_map = std::shared_ptr<UVMap<size_t>>(new ConstantUVMap<size_t>(0));

            // Default values for all pose information:
            this -> scale = 1;
            this -> position = bvh::Vector3<Scalar>(0,0,0);
            this -> rotation[0][0] = 1;
            this -> rotation[0][1] = 0;
            this -> rotation[0][2] = 0;
            this -> rotation[1][0] = 0;
            this -> rotation[1][1] = 1;
            this -> rotation[1][2] = 0;
            this -> rotation[2][0] = 0;
            this -> rotation[2][1] = 0;
            this -> rotation[2][2] = 1;
        }

        void set_id(uint32_t id){
            this->id = id;
        }

        void set_scale(Scalar scale){
            this -> scale = scale;
            resize_triangles(this->triangles, scale, this->position);
        }

        void set_position(bvh::Vector3<Scalar> position) {
            this -> position = position;
            translate_triangles(this->triangles, position);
        }

        void set_rotation(Scalar rotation[3][3]) {
            for (int i = 0; i < 3; i++){
                for (int j = 0; j <3; j++){
                    this -> rotation[i][j] = rotation[i][j];
                }
            }

            // Apply rotation (remove and re-apply translation)
            rotate_triangles(this->triangles, rotation, this->position);
        }

        void set_pose(bvh::Vector3<Scalar> position, Scalar rotation[3][3]){
            set_rotation(rotation);
            set_position(position);
        }

        const std::vector<bvh::Triangle<Scalar>> get_triangles() {
            return triangles;
        }

        Material<Scalar>* get_material(float u, float v) {
            return materials[(*material_map)(u, v)].get();
        }

};

#endif