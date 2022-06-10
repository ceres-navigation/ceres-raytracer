#ifndef __TRANSFORM_H
#define __TRANSFORM_H

#include <bvh/bvh.hpp>
#include <bvh/triangle.hpp>


template <typename Scalar>
bvh::Vector3<Scalar> resize(bvh::Vector3<Scalar> vector, Scalar scale){
    vector[0] = scale*vector[0];
    vector[1] = scale*vector[1];
    vector[2] = scale*vector[2];
    return vector;
}

template <typename Scalar>
bvh::Vector3<Scalar> translate(bvh::Vector3<Scalar> vector, bvh::Vector3<Scalar> position){
    return bvh::Vector3<Scalar>(vector[0] + position[0], vector[1] + position[1], vector[2] + position[2]);
}

template <typename Scalar>
bvh::Vector3<Scalar> rotate(bvh::Vector3<Scalar> vector, Scalar rotation[3][3]){
    return bvh::Vector3<Scalar>(
        rotation[0][0]*vector[0] + rotation[0][1]*vector[1] + rotation[0][2]*vector[2],
        rotation[1][0]*vector[0] + rotation[1][1]*vector[1] + rotation[1][2]*vector[2],
        rotation[2][0]*vector[0] + rotation[2][1]*vector[1] + rotation[2][2]*vector[2]
    );
}

template <typename Scalar>
bvh::Vector3<Scalar> transform(bvh::Vector3<Scalar> vector, Scalar rotation[3][3], bvh::Vector3<Scalar> position, Scalar scale){
    vector[0] = scale*vector[0];
    vector[1] = scale*vector[1];
    vector[2] = scale*vector[2];
    return bvh::Vector3<Scalar>(
        rotation[0][0]*vector[0] + rotation[0][1]*vector[1] + rotation[0][2]*vector[2] + position[0],
        rotation[1][0]*vector[0] + rotation[1][1]*vector[1] + rotation[1][2]*vector[2] + position[1],
        rotation[2][0]*vector[0] + rotation[2][1]*vector[1] + rotation[2][2]*vector[2] + position[2]
    );
}

template <typename Scalar>
void resize_triangles(std::vector<bvh::Triangle<Scalar>> &triangles, Scalar scale){
    for (auto &tri : triangles) {
        // Scale each of the vertices:
        auto p0 = resize(tri.p0,   scale);
        auto p1 = resize(tri.p1(), scale);
        auto p2 = resize(tri.p2(), scale);

        // Update the triangle:
        tri.update_vertices(p0,p1,p2);
    }
}

template <typename Scalar>
void translate_triangles(std::vector<bvh::Triangle<Scalar>> &triangles, bvh::Vector3<Scalar> position){
    for (auto &tri : triangles) {
        // Transform each of the vertices:
        auto p0 = translate(tri.p0,   position);
        auto p1 = translate(tri.p1(), position);
        auto p2 = translate(tri.p2(), position);

        // Update the triangle:
        tri.update_vertices(p0,p1,p2);
    }
}

template <typename Scalar>
void rotate_triangles(std::vector<bvh::Triangle<Scalar>> &triangles, Scalar rotation[3][3]){
    for (auto &tri : triangles) {
        // Rotate each of the vertices:
        auto p0 = rotate(tri.p0,   rotation);
        auto p1 = rotate(tri.p1(), rotation);
        auto p2 = rotate(tri.p2(), rotation);

        // Transform each of the vertex normals:
        auto vn0 = rotate(tri.vn0, rotation);
        auto vn1 = rotate(tri.vn1, rotation);
        auto vn2 = rotate(tri.vn2, rotation);

        // Update the triangle:
        tri.update_vertices(p0,p1,p2);
        tri.update_vertex_normals(vn0, vn1, vn2);
    }
}

#endif