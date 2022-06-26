#ifndef __SIMPLE_LIDAR_H
#define __SIMPLE_LIDAR_H

#include <bvh/bvh.hpp>

template <typename Scalar>
class SimpleLidar: public Lidar<Scalar> {
    public:
        bool z_positive;

    SimpleLidar(bool z_positive){
        this -> z_positive = z_positive;
    };

    // Copy constructor:
    SimpleLidar(const SimpleLidar<Scalar> &original) : Lidar<Scalar>(original){
        this -> z_positive = original.z_positive;

        //TODO: move to RigidBody copy:
        this -> position = original.position;
        for (auto i = 0; i < 3; i++){
            for (auto j = 0; j < 3; j++) {
                this->rotation[i][j] = original.rotation[i][j];
            }
        }        
    }

    std::vector<bvh::Ray<Scalar>> cast_rays(int num_rays){
        std::vector<bvh::Ray<Scalar>> rays;

        bvh::Vector3<Scalar> dir;
        if (this->z_positive){
            dir = bvh::normalize(bvh::Vector3<Scalar>(0, 0, 1));
        }
        else {
            dir = bvh::normalize(bvh::Vector3<Scalar>(0, 0, -1));
        }

        // Apply rotation of the beam:
        bvh::Vector3<Scalar> temp;
        temp[0] = this->rotation[0][0]*dir[0] + this->rotation[1][0]*dir[1] + this->rotation[2][0]*dir[2];
        temp[1] = this->rotation[0][1]*dir[0] + this->rotation[1][1]*dir[1] + this->rotation[2][1]*dir[2];
        temp[2] = this->rotation[0][2]*dir[0] + this->rotation[1][2]*dir[1] + this->rotation[2][2]*dir[2];
        dir = temp;
        
        bvh::Ray<Scalar> ray(this->position, dir);
        rays.push_back(ray);

        return rays;
    }
};

#endif