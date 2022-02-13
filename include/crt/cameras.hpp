#ifndef __CAMERAS_H
#define __CAMERAS_H

#include "bvh/bvh.hpp"
// #include "bvh/vector.hpp"
// #include "bvh/ray.hpp"

template <typename Scalar>
struct CameraStruct {
    std::string name;
    bvh::Vector3<Scalar> position;
    Scalar rotation[3][3];
    Scalar focal_length;
    Scalar resolution[2];
    Scalar sensor_size[2];
} ;

template <typename Scalar>
class CameraModel {
    using Vector3 =  bvh::Vector3<Scalar>;
    public:
        Vector3 position;
        Scalar rotation[3][3];

        Scalar focal_length;
        Scalar resolution[2];
        Scalar sensor_size[2];
        
        Scalar center[2];
        Scalar scale[2];
        Scalar K[3][3];
        virtual bvh::Ray<Scalar> pixel_to_ray(Scalar u, Scalar v) = 0;

        virtual Scalar get_resolutionX() = 0;
        virtual Scalar get_resolutionY() = 0;
};

template <typename Scalar>
class PinholeCamera: public CameraModel<Scalar> {
    using Vector3 =  bvh::Vector3<Scalar>;
    public:
        Vector3 position;
        Scalar rotation[3][3];

        Scalar focal_length;
        Scalar resolution[2];
        Scalar sensor_size[2];
        
        Scalar center[2];
        Scalar scale[2];
        Scalar K[3][3];

        PinholeCamera(CameraStruct<Scalar> camera_struct);

        Scalar get_resolutionX();

        Scalar get_resolutionY();

        bvh::Ray<Scalar> pixel_to_ray(Scalar u, Scalar v);
};

#endif