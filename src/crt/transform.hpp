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
void resize_triangles(std::vector<bvh::Triangle<Scalar>> &triangles, Scalar scale, bvh::Vector3<Scalar> position){
    for (auto &tri : triangles) {
        // Undo position:
        auto p0 = translate(tri.p0,   -position);
        auto p1 = translate(tri.p1(), -position);
        auto p2 = translate(tri.p2(), -position);

        // Scale each of the vertices:
        p0 = resize(p0, scale);
        p1 = resize(p1, scale);
        p2 = resize(p2, scale);

        // Reapply position:
        p0 = translate(p0, position);
        p1 = translate(p1, position);
        p2 = translate(p2, position);

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
void rotate_triangles(std::vector<bvh::Triangle<Scalar>> &triangles, Scalar rotation[3][3], bvh::Vector3<Scalar> position){
    for (auto &tri : triangles) {
        // Undo position:
        auto p0 = translate(tri.p0,   -position);
        auto p1 = translate(tri.p1(), -position);
        auto p2 = translate(tri.p2(), -position);

        // Rotate each of the vertices:
        p0 = rotate(p0, rotation);
        p1 = rotate(p1, rotation);
        p2 = rotate(p2, rotation);

        // Reapply position:
        p0 = translate(p0, position);
        p1 = translate(p1, position);
        p2 = translate(p2, position);

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