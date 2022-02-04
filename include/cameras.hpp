#ifndef __CAMERAS_H
#define __CAMERAS_H

#include <bvh/bvh.hpp>

template <typename Scalar>
class CameraModel {
    public:
        virtual void pixel_to_ray();
};

template <typename Scalar>
class PinholeCamera {
    using Vector3 =  bvh::Vector3<Scalar>;
    public:
        Vector3 position;
        Vector3 euler_angles;

        Scalar focal_length;
        Scalar resolution[2];
        Scalar sensor_size[2];
        
        Scalar center[2];
        Scalar scale[2];
        Scalar K[3][3];

        PinholeCamera(Scalar focal_length, Scalar resolution[2], Scalar sensor_size[2],
                      Vector3 position, Vector3 euler_angles) {
            position = position;
            euler_angles = euler_angles;

            focal_length = focal_length;
            resolution = resolution;
            sensor_size = sensor_size;

            center[0] = resolution[0]/2.0;
            center[1] = resolution[1]/2.0;
            scale[0] = resolution[0]/sensor_size[0];
            scale[1] = resolution[1]/sensor_size[1];
            K[0][0] = focal_length;
            K[0][1] = 0;
            K[0][2] = center[0];
            K[1][0] = 0;
            K[1][1] = focal_length;
            K[1][2] = center[1];
            K[2][0] = 0;
            K[2][1] = 0;
            K[2][2] = 1;
        }

        bvh::Ray<Scalar> pixel_to_ray(Scalar u, Scalar v) {
            Vector3 dir = Vector3((-center[0]+u)/scale[0], (center[1]-v)/scale[1], -focal_length);
            bvh::Ray<Scalar> ray(position, bvh::normalize(dir));
            return ray;
        }
};

#endif