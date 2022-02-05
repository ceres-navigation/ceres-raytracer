#ifndef __LIGHTS_H
#define __LIGHTS_H

#include <bvh/bvh.hpp>

// Abstract light class:
template <typename Scalar>
class Light {
    using Vector3 =  bvh::Vector3<Scalar>;
    public:
        virtual bvh::Ray<Scalar> sample_ray(Vector3 origin) = 0;
};


// Point light class:
template <typename Scalar>
class PointLight: public Light<Scalar>  {
    using Vector3 =  bvh::Vector3<Scalar>;
    public:
        Vector3 position;

        PointLight(Vector3 position){
            this -> position = position;
        }

        bvh::Ray<Scalar> sample_ray(Vector3 origin){
            bvh::Vector3<Scalar> light_direction = bvh::normalize(this->position - origin);
            return bvh::Ray<Scalar>(origin, light_direction);
        }
};

#endif