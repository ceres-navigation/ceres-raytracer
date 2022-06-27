#ifndef __SIMPLE_LIDAR_H
#define __SIMPLE_LIDAR_H

#include <bvh/bvh.hpp>

template <typename Scalar>
class SimpleLidar: public Lidar<Scalar> {
    public:

    SimpleLidar(bool z_positive){
        this -> z_positive = z_positive;

    };

    // Copy constructor:
    SimpleLidar(const SimpleLidar<Scalar> &original) : Lidar<Scalar>(original){
        this -> z_positive = original.z_positive;

        // Copy the batch data:
        this->batch_positions.clear();
        for (auto current_position : original.batch_positions){
            this->batch_positions.push_back(current_position);
        }

        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++){
                for (int k = 0; k < 100000; k++){
                    this->batch_rotations[i][j][k] = original.batch_rotations[i][j][k];
                }
            }
        }

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

    std::vector<std::vector<bvh::Ray<Scalar>>> batch_cast_rays(int num_rays){
        std::vector<std::vector<bvh::Ray<Scalar>>> batch_rays;

        int count = 0;
        for (auto current_position : this->batch_positions){
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
            temp[0] = this->batch_rotations[0][0][count]*dir[0] + this->batch_rotations[1][0][count]*dir[1] + this->batch_rotations[2][0][count]*dir[2];
            temp[1] = this->batch_rotations[0][1][count]*dir[0] + this->batch_rotations[1][1][count]*dir[1] + this->batch_rotations[2][1][count]*dir[2];
            temp[2] = this->batch_rotations[0][2][count]*dir[0] + this->batch_rotations[1][2][count]*dir[1] + this->batch_rotations[2][2][count]*dir[2];
            dir = temp;
            
            bvh::Ray<Scalar> ray(current_position, dir);
            rays.push_back(ray);

            // Add rays to batch:
            batch_rays.push_back(rays);
            count++;
        }

        return batch_rays;
    }

    void batch_set_pose(std::vector<bvh::Vector3<Scalar>> batch_positions_new, Scalar batch_rotations[3][3][100000]){
        this->batch_positions.clear();
        for (auto current_position : batch_positions_new) {
            this->batch_positions.push_back(current_position);
        }

        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++){
                for (int k = 0; k < 100000; k++){
                    this->batch_rotations[i][j][k] = batch_rotations[i][j][k];
                }
            }
        }
    }
};

#endif