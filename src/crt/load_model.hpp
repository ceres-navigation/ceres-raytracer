#ifndef __LOAD_MODEL_H
#define __LOAD_MODEL_H

#include "bvh/bvh.hpp"
#include "bvh/triangle.hpp"
#include "bvh/vector.hpp"

#include "entity.hpp"
#include "materials/material.hpp"

template <typename Scalar>
std::vector<Entity<Scalar>> load_model(std::string path_to_model, bool smooth, bvh::Vector3<Scalar> position, Scalar scale, Scalar rotation[3][3]){
    std::vector<Entity<Scalar>> new_entities;
    std::shared_ptr<Material<Scalar>> material(nullptr);
    std::shared_ptr<UVMap<size_t>> material_map;
    std::shared_ptr<UVMap<Color>> texture(nullptr);
    std::vector<bvh::Triangle<Scalar>> triangles;

    // Load the mesh geometry:
    std::string extension = std::filesystem::path(path_to_model).extension();
    std::transform(extension.begin(), extension.end(), extension.begin(), static_cast<int(*)(int)>(std::tolower));
    if (extension.compare(".obj") == 0) {
        triangles = obj::load_from_file<Scalar>(path_to_model);
    } 
    else { 
        std::cout << "file type of " << extension << " is not a valid.  ceres-rt only supports .obj/.OBJ\n";
    }
    std::cout << "  " << path_to_model << " provided " << triangles.size() << " triangles\n";

    Entity<Scalar> entity(triangles, smooth);

    new_entities.push_back(entity);
    return new_entities;
}

#endif