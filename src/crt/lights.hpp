#ifndef __LIGHTS_H
#define __LIGHTS_H

#include <random>

#include <bvh/bvh.hpp>

#include "transform.hpp"

// Abstract light class:
template <typename Scalar>
class Light {
    public:
        Scalar intensity;
        bvh::Vector3<Scalar> position;
        Scalar rotation[3][3];

        // Abstract methods:
        virtual bvh::Ray<Scalar> sample_ray(bvh::Vector3<Scalar> origin) = 0;
        virtual Scalar get_intensity(bvh::Vector3<Scalar> point) = 0;

        void set_position(bvh::Vector3<Scalar> position) {
            this -> position = position;
        }

        void set_rotation(Scalar rotation[3][3]) {
            for (int i = 0; i < 3; i++){
                for (int j = 0; j <3; j++){
                    this -> rotation[i][j] = rotation[i][j];
                }
            }
        }

        void set_pose(bvh::Vector3<Scalar>  position, Scalar rotation[3][3]){
            set_position(position);
            set_rotation(rotation);
        }

        void translate(bvh::Vector3<Scalar> translation){
            this -> position = position + translation;
        }

        void rotate_about_origin(Scalar new_rotation[3][3]){
            this -> rotation = multiply_rotations(this->rotation, new_rotation);
            this -> position = rotate_vector(this->position, new_rotation);
        }
};


// Point light class:
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

template <typename Scalar>
class SquareLight: public Light<Scalar> {
    public:
        Scalar size[2];
        bvh::Vector3<Scalar> sampled_point;

        std::mt19937 generator;
        std::uniform_real_distribution<Scalar> distr_x;
        std::uniform_real_distribution<Scalar> distr_y;

        SquareLight(Scalar intensity, Scalar size[2]) {
            this->intensity = intensity;
            this->size[0] = size[0];
            this->size[1] = size[1];

            std::random_device rand_dev;
            std::mt19937 generator(rand_dev());
            std::uniform_real_distribution<Scalar> distr_x(0.0,1.0);
            std::uniform_real_distribution<Scalar> distr_y(0.0,1.0);

            // Default pose information:
            this -> position = bvh::Vector3<Scalar>(0,0,0);
            this -> rotation[0][0] = 1;
            this -> rotation[0][1] = 0;
            this -> rotation[0][2] = 0;
            this -> rotation[1][0] = 0;
            this -> rotation[1][1] = 1;
            this -> rotation[1][2] = 0;
            this -> rotation[2][0] = 0;
            this -> rotation[2][1] = 0;
            this -> rotation[2][2] = 1;
        };

        // Copy constructor:
        SquareLight(const SquareLight<Scalar> &rhs) {
            this -> intensity = rhs.intensity;
            this -> position  = rhs.position;
            for (auto i = 0; i < 3; i++){
                for (auto j = 0; j < 3; j++){
                    this -> rotation[i][j] = rhs.rotation[i][j];
                }
            }
        }

        bvh::Ray<Scalar> sample_ray(bvh::Vector3<Scalar> origin){
            // Select a random point on the light:
            Scalar x_coord = (this->size[0])*distr_x(generator) - (this->size[0]/2);
            Scalar y_coord = (this->size[1])*distr_y(generator) - (this->size[1]/2);
            bvh::Vector3<Scalar> point_on_light(x_coord, y_coord, 0.);

            Scalar scale = 1.0;

            // Transform the point to world coordinates:
            this->sampled_point = transform(point_on_light, this->rotation, this->position, scale);

            // Generate the ray:
            bvh::Vector3<Scalar> light_direction = bvh::normalize(this->sampled_point - origin);
            return bvh::Ray<Scalar>(origin, light_direction, 0, bvh::length(this->sampled_point - origin));
        };

        Scalar get_intensity(bvh::Vector3<Scalar> point) { 
            return this->intensity / bvh::dot(point - this->sampled_point, point - this->sampled_point);
        };
};

#endif