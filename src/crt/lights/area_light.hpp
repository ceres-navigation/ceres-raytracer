#ifndef __AREA_LIGHT_H
#define __AREA_LIGHT_H

template <typename Scalar>
class AreaLight: public Light<Scalar> {
    public:
        Scalar size[2];
        bvh::Vector3<Scalar> sampled_point;

        std::mt19937 generator;
        std::uniform_real_distribution<Scalar> distr_x;
        std::uniform_real_distribution<Scalar> distr_y;

        AreaLight(Scalar intensity, Scalar size[2]) {
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
        AreaLight(const AreaLight<Scalar> &original) {
            this -> intensity = original.intensity;
            this -> size[0] = original.size[0];
            this -> size[1] = original.size[1];
            this -> generator = original.generator;
            this -> distr_x = original.distr_x;
            this -> distr_y = original.distr_y;

            this -> position  = original.position;
            for (auto i = 0; i < 3; i++){
                for (auto j = 0; j < 3; j++){
                    this -> rotation[i][j] = original.rotation[i][j];
                }
            }
        };

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