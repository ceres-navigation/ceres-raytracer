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
};


// Point light class:
template <typename Scalar>
class PointLight: public Light<Scalar>  {
    public:
        bvh::Vector3<Scalar> position;

        PointLight(bvh::Vector3<Scalar> position){
            this -> position = position;
        };

        bvh::Ray<Scalar> sample_ray(bvh::Vector3<Scalar> origin){
            bvh::Vector3<Scalar> light_direction = bvh::normalize(this->position - origin);
            return bvh::Ray<Scalar>(origin, light_direction);
        };
};

template <typename Scalar>
class SquareLight: public Light<Scalar> {
    public:
        bvh::Vector3<Scalar> position;
        Scalar rotation[3][3];
        Scalar size[2];

        std::mt19937 generator;
        std::uniform_real_distribution<Scalar> distr_x;
        std::uniform_real_distribution<Scalar> distr_y;

        SquareLight(bvh::Vector3<Scalar> position, Scalar rotation[3][3], Scalar size[2]) {
            this -> position = position;
            for (int i = 0; i < 3; ++i){
                for (int j = 0; j < 3; ++j) {
                    this -> rotation[i][j] = rotation[i][j];
                };
            };
            this -> size[0] = size[0];
            this -> size[1] = size[1];

            std::random_device rand_dev;
            std::mt19937 generator(rand_dev());
            std::uniform_real_distribution<Scalar> distr_x(-size[0]/2, size[0]/2);
            std::uniform_real_distribution<Scalar> distr_y(-size[1]/2, size[1]/2);
        };

        bvh::Ray<Scalar> sample_ray(bvh::Vector3<Scalar> origin){
            // Select a random point on the light:
            Scalar x_coord = distr_x(generator);
            Scalar y_coord = distr_y(generator);
            bvh::Vector3<Scalar> point_on_light(x_coord, y_coord, 0);

            Scalar scale = 1.0;

            // Transform the point to world coordinates:
            bvh::Vector3<Scalar> point_on_light_world = transform(point_on_light, this->rotation, this->position, scale);

            // Generate the ray:
            bvh::Vector3<Scalar> light_direction = bvh::normalize(point_on_light_world - origin);
            return bvh::Ray<Scalar>(origin, light_direction);
        };
};

#endif