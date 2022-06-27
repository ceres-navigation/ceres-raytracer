#ifndef __LIDAR_H
#define __LIDAR_H

#include <bvh/bvh.hpp>

template <typename Scalar>
class Lidar: public RigidBody<Scalar> {
    public:
        bool z_positive;
        std::vector<bvh::Vector3<Scalar>> batch_positions;
        Scalar batch_rotations[3][3][100000];

        virtual std::vector<bvh::Ray<Scalar>> cast_rays(int num_rays) = 0;

        virtual std::vector<std::vector<bvh::Ray<Scalar>>> batch_cast_rays(int num_rays) = 0;
        virtual void batch_set_pose(std::vector<bvh::Vector3<Scalar>> batch_positions, 
                                    Scalar batch_rotations[3][3][100000]) = 0;
};

#endif