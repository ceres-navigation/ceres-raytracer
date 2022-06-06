#ifndef __LOAD_MODEL_H
#define __LOAD_MODEL_H

#include "bvh/bvh.hpp"
#include "bvh/triangle.hpp"
#include "bvh/vector.hpp"

#include "entity.hpp"
#include "materials/material.hpp"

template <typename Scalar>
std::vector<Entity<Scalar>*> load_model(std::string path_to_model, bool smooth, bvh::Vector3<Scalar> position, Scalar scale, Scalar rotation[3][3], Color color){
    std::vector<Entity<Scalar>*> new_entities;
    std::shared_ptr<Material<Scalar>> material(nullptr);
    std::shared_ptr<UVMap<size_t>> material_map;
    std::shared_ptr<UVMap<Color>> texture(nullptr);
    std::vector<bvh::Triangle<Scalar>> triangles;

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
    std::cout << "  " << path_to_model << " provided " << triangles.size() << " triangles\n";

    // Apply the transformations to the loaded models:
    for (auto &tri : new_triangles) {
        // Transform each of the vertices:
        auto p0 = transform(tri.p0, rotation, position, scale);
        auto p1 = transform(tri.p1(), rotation, position, scale);
        auto p2 = transform(tri.p2(), rotation, position, scale);

        // Transform each of the vertex normals:
        auto vn0 = rotate(tri.vn0, rotation);
        auto vn1 = rotate(tri.vn1, rotation);
        auto vn2 = rotate(tri.vn2, rotation);

        // Ok, somebody needs to fix the copy constructor...
        auto vc0 = tri.vc[0];
        auto vc1 = tri.vc[1];
        auto vc2 = tri.vc[2];

        auto uv0 = tri.uv[0];
        auto uv1 = tri.uv[1];
        auto uv2 = tri.uv[2];

        triangles.emplace_back(p0, p1, p2);
        triangles.rbegin()->add_vetex_normals(vn0, vn1, vn2);
        triangles.rbegin()->add_vertex_colors(vc0, vc1, vc2);
        triangles.rbegin()->add_vertex_uv(uv0, uv1, uv2);
    }

    // Create a new entity instance:
    Entity<Scalar>* entity = new Entity<Scalar>(triangles, path_to_model, smooth, color);
    new_entities.push_back(entity);
    return new_entities;
}

#endif