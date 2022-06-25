#ifndef __CAMERA_H
#define __CAMERA_H

#include <bvh/bvh.hpp>
#include "rigid_body.hpp"

template <typename Scalar>
class Camera: public RigidBody<Scalar> {
    public:
        bool z_positive;
        Scalar focal_length;  

        virtual ~Camera() {}

        virtual bvh::Ray<Scalar> pixel_to_ray(Scalar u, Scalar v) = 0;

        // Additional information:
        // Aperture aperture;

        // TODO: move to sensor class...
        // Sensor sensor;
        Scalar scale[2];
        Scalar center[2];
        Scalar resolution[2];
        Scalar sensor_size[2];
        virtual Scalar get_resolutionX() = 0;
        virtual Scalar get_resolutionY() = 0;
};

#endif