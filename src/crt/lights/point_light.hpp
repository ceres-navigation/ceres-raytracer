#ifndef __POINT_LIGHT_H
#define __POINT_LIGHT_H

#include <bvh/bvh.hpp>

template <typename Scalar>
class PointLight: public Light<Scalar>  {
    public:
        PointLight(Scalar intensity) { 
            this -> intensity = intensity;

            // Default pose information:
            this -> position = bvh::Vector3<Scalar>(0,0,0);
        };

        // Copy constructor:
        PointLight(const PointLight<Scalar> &rhs) {
            this -> intensity = rhs.intensity;
            this -> position  = rhs.position;

            this -> rotation[0][0] = 1;
            this -> rotation[0][1] = 0;
            this -> rotation[0][2] = 0;
            this -> rotation[1][0] = 0;
            this -> rotation[1][1] = 1;
            this -> rotation[1][2] = 0;
            this -> rotation[2][0] = 0;
            this -> rotation[2][1] = 0;
            this -> rotation[2][2] = 1;
        }

        bvh::Ray<Scalar> sample_ray(bvh::Vector3<Scalar> origin){
            bvh::Vector3<Scalar> light_direction = bvh::normalize(this->position - origin);
            return bvh::Ray<Scalar>(origin, light_direction, 0, bvh::length(this->position - origin));
        };

        Scalar get_intensity(bvh::Vector3<Scalar> point) { 
            return std::min(this->intensity / bvh::dot(point - this->position, point - this->position), Scalar(10000));
        };
};

#endif