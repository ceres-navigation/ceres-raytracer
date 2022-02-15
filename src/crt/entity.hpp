#ifndef __ENTITY_H
#define __ENTITY_H

#include <filesystem>
#include <memory>
#include <vector>
#include <random>

#include <Magick++.h>

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
        std::vector<bvh::Triangle<Scalar>> triangles;
        std::vector<std::shared_ptr<Material<Scalar>>> materials;
        std::shared_ptr<UVMap<size_t>> material_map;
        bool smooth_shading;

        Entity(std::vector<bvh::Triangle<Scalar>> triangles, bool smooth_shading) {
            // Set current entity as the parent object for all input triangles:
            for (auto &tri : triangles) {
                tri.set_parent(this);
                this -> triangles.push_back(tri);
            }
            this->smooth_shading = smooth_shading;

            //TODO: REMOVE ALL OF THE HARDCODED STUFF HERE:
            auto texture = std::shared_ptr<UVMap<Color>>(new ConstantUVMap<Color>(Color(0.5,0.5,0.5)));
            Color color(0.5,0.5,0.5);
            this->materials.emplace_back(
                    new TexturedBlinnPhongMaterial<Scalar>(
                        std::shared_ptr<UVMap<Color>>(new ConstantUVMap<Color>(color)), 
                        std::shared_ptr<UVMap<Color>>(new ConstantUVMap<Color>(Color(0,0.5,0.8))),
                        32
                    )
                );
            this->material_map = std::shared_ptr<UVMap<size_t>>(new ConstantUVMap<size_t>(0));
        }

        const std::vector<bvh::Triangle<Scalar>> get_triangles() {
            return triangles;
        }

        Material<Scalar> *get_material(float u, float v) {
            return materials[(*material_map)(u, v)].get();
        }

};

#endif