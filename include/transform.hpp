#ifndef __TRANSFORM_H
#define __TRANSFORM_H

#include <bvh/bvh.hpp>
#include <bvh/triangle.hpp>

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
bvh::Vector3<Scalar> rotate(bvh::Vector3<Scalar> vector, Scalar rotation[3][3]){
    return bvh::Vector3<Scalar>(
        rotation[0][0]*vector[0] + rotation[0][1]*vector[1] + rotation[0][2]*vector[2],
        rotation[1][0]*vector[0] + rotation[1][1]*vector[1] + rotation[1][2]*vector[2],
        rotation[2][0]*vector[0] + rotation[2][1]*vector[1] + rotation[2][2]*vector[2]
    );
}

template <typename Scalar>
void transform_triangles(std::vector<bvh::Triangle<Scalar>> &triangles, Scalar rotation[3][3], bvh::Vector3<Scalar> position, Scalar scale) {
    // Apply the rotation and translation:
    // #pragma omp parallel for
    for (size_t i = 0; i < triangles.size(); ++i) {
        // Transform each of the vertices:
        auto p0 = transform(triangles[i].p0, rotation, position, scale);
        auto p1 = transform(triangles[i].p1(), rotation, position, scale);
        auto p2 = transform(triangles[i].p2(), rotation, position, scale);

        // Transform each of the vertex normals:
        auto vn0 = rotate(triangles[i].vn0, rotation);
        auto vn1 = rotate(triangles[i].vn1, rotation);
        auto vn2 = rotate(triangles[i].vn2, rotation);

        // Update the triangle:
        triangles[i] = bvh::Triangle<Scalar>(p0, p1, p2);
        triangles[i].add_vetex_normals(vn0, vn1, vn2);
    }
}

#endif