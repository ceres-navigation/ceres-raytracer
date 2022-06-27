#ifndef __GAUSSIAN_BEAM_LIDAR_H
#define __GAUSSIAN_BEAM_LIDAR_H

#include <bvh/bvh.hpp>

template <typename Scalar>
class GaussianBeamLidar: public Lidar<Scalar> {
    public:
        Scalar angular_spread;
        Scalar beam_waist;

    GaussianBeamLidar(Scalar angular_spread, Scalar beam_waist, bool z_positive){
        this -> angular_spread = angular_spread;
        this -> beam_waist = beam_waist;
        this -> z_positive;
    };

    std::vector<bvh::Ray<Scalar>> cast_rays(int num_rays){
        std::vector<bvh::Ray<Scalar>> rays;

        bvh::Vector3<Scalar> dir;
        if (this->z_positive){
            dir = bvh::normalize(Vector3(0, 0, this->focal_length));
        }
        else {
            dir = bvh::normalize(Vector3(0, 0, -this->focal_length));
        }

        // Apply rotation of the beam:
        Vector3 temp;
        temp[0] = this->rotation[0][0]*dir[0] + this->rotation[1][0]*dir[1] + this->rotation[2][0]*dir[2];
        temp[1] = this->rotation[0][1]*dir[0] + this->rotation[1][1]*dir[1] + this->rotation[2][1]*dir[2];
        temp[2] = this->rotation[0][2]*dir[0] + this->rotation[1][2]*dir[1] + this->rotation[2][2]*dir[2];
        dir = temp;
        
        auto ray = bvh::Ray<Scalar> ray(this->position, dir);
        rays.push_back(ray);

        return rays;
    }
};

#endif