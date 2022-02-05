#ifndef __LIGHTS_H
#define __LIGHTS_H

#include <bvh/bvh.hpp>

// Abstract light class:
template <typename Scalar>
class Light {
    using Vector3 =  bvh::Vector3<Scalar>;
    public:
        Vector3 position;

        virtual bvh::Ray<Scalar> sample_ray(Scalar position) = 0;
};


// Point light class:
template <typename Scalar>
class PointLight: public Light<Scalar>  {
    using Vector3 =  bvh::Vector3<Scalar>;
    public:
        Vector3 position;

        PointLight(){

        }
};


// Area light class:
template <typename Scalar>
class AreaLight: public Light<Scalar> {
    using Vector3 =  bvh::Vector3<Scalar>;
    public:
        Vector3 position;
        Scalar rotation[3][3];
        int num_samples;
};

#endif