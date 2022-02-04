#ifndef __CAMERAS_H
#define __CAMERAS_H

#include <bvh/bvh.hpp>

template <typename Scalar>
struct Camera {
    bvh::Vector3<Scalar> eye;
    bvh::Vector3<Scalar> dir;
    bvh::Vector3<Scalar> up;
    Scalar  fov;
};

#endif