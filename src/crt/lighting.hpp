#ifndef __LIGHTS_H
#define __LIGHTS_H

#include <random>

#include <bvh/bvh.hpp>

#include "transform.hpp"

// Abstract light class:
template <typename Scalar>
class Light {
    public:
        virtual bvh::Ray<Scalar> sample_ray(bvh::Vector3<Scalar> origin) = 0;
        virtual Scalar get_intensity() = 0;
};


// Point light class:
template <typename Scalar>
class PointLight: public Light<Scalar>  {
    public:
        bvh::Vector3<Scalar> position;
        Scalar intensity;

        PointLight(bvh::Vector3<Scalar> position, Scalar intensity = 1.0) 
        : position(position), intensity(intensity) { };

        bvh::Ray<Scalar> sample_ray(bvh::Vector3<Scalar> origin){
            bvh::Vector3<Scalar> light_direction = bvh::normalize(position - origin);
            return bvh::Ray<Scalar>(origin, light_direction, 0, bvh::length(position - origin));
        };

        Scalar get_intensity() { return intensity; }
        Scalar get_intensity(bvh::Vector3<Scalar> point) { 
            return std::min(intensity / bvh::dot(point - position, point - position), Scalar(10000));
        }
};

template <typename Scalar>
class SquareLight: public Light<Scalar> {
    public:
        bvh::Vector3<Scalar> position;
        Scalar rotation[3][3];
        Scalar size[2];
        Scalar intensity;
        bvh::Vector3<Scalar> sampled_point;

        std::mt19937 generator;
        std::uniform_real_distribution<Scalar> distr_x;
        std::uniform_real_distribution<Scalar> distr_y;

        SquareLight(bvh::Vector3<Scalar> position, Scalar rotation[3][3], Scalar size[2], Scalar intensity = 1.) {
            this->position = position;
            for (int i = 0; i < 3; ++i){
                for (int j = 0; j < 3; ++j) {
                    this -> rotation[i][j] = rotation[i][j];
                };
            };
            this->size[0] = size[0];
            this->size[1] = size[1];

            this->intensity = intensity;

            std::random_device rand_dev;
            std::mt19937 generator(rand_dev());
            std::uniform_real_distribution<Scalar> distr_x(0.0,1.0);
            std::uniform_real_distribution<Scalar> distr_y(0.0,1.0);
        };

        bvh::Ray<Scalar> sample_ray(bvh::Vector3<Scalar> origin){
            // Select a random point on the light:
            Scalar x_coord = (this->size[0])*distr_x(generator) - (this->size[0]/2);
            Scalar y_coord = (this->size[1])*distr_y(generator) - (this->size[1]/2);
            bvh::Vector3<Scalar> point_on_light(x_coord, y_coord, 0.);

            Scalar scale = 1.0;

            // Transform the point to world coordinates:
            this->sampled_point = transform(point_on_light, this->rotation, this->position, scale);

            // std::cout<<sampled_point[0]<<" "<<sampled_point[1]<<" "<<sampled_point[2]<<"\n";

            // Generate the ray:
            bvh::Vector3<Scalar> light_direction = bvh::normalize(this->sampled_point - origin);
            return bvh::Ray<Scalar>(origin, light_direction, 0, bvh::length(this->sampled_point - origin));
        };

        Scalar get_intensity() {
            return intensity; 
        };

        Scalar get_intensity(bvh::Vector3<Scalar> point) { 
            return intensity / bvh::dot(point - this->sampled_point, point - this->sampled_point);
        };
};

#endif