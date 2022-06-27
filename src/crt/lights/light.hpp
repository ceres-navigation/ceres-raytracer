#ifndef __LIGHT_H
#define __LIGHT_H

#include <random>

#include <bvh/bvh.hpp>

#include "transform.hpp"

// Abstract light class:
template <typename Scalar>
class Light: public RigidBody<Scalar> {
    public:
        Scalar intensity;

        // Abstract methods:
        virtual bvh::Ray<Scalar> sample_ray(bvh::Vector3<Scalar> origin) = 0;
        virtual Scalar get_intensity(bvh::Vector3<Scalar> point) = 0;
};

#endif