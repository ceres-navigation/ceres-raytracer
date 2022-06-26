#ifndef __LIDAR_H
#define __LIDAR_H

#include <bvh/bvh.hpp>

template <typename Scalar>
class Lidar: public RigidBody<Scalar> {
    public:
        virtual std::vector<bvh::Ray<Scalar>> cast_rays(int num_rays) = 0;
};

#endif