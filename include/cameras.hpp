#ifndef __CAMERAS_H
#define __CAMERAS_H

#include <bvh/bvh.hpp>

struct CameraStruct {
    std::string name;
    bvh::Vector3<double> position;
    double rotation[3][3];
    double focal_length;
    double resolution[2];
    double sensor_size[2];
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

        PinholeCamera(CameraStruct camera_struct) {
            this -> position = Vector3((Scalar) camera_struct.position[0],
                                       (Scalar) camera_struct.position[1],
                                       (Scalar) camera_struct.position[2]);

            for (int i = 0; i < 3; i++){
                for (int j = 0; j <3; j++){
                    this -> rotation[i][j] = camera_struct.rotation[i][j];
                }
            }

            this -> focal_length = (Scalar) camera_struct.focal_length;
            this -> resolution[0] = (Scalar) camera_struct.resolution[0];
            this -> resolution[1] = (Scalar) camera_struct.resolution[1];
            this -> sensor_size[0] = (Scalar) camera_struct.sensor_size[0];
            this -> sensor_size[1] = (Scalar) camera_struct.sensor_size[1];

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

        Scalar get_resolutionX() {
            return resolution[0];
        }

        Scalar get_resolutionY() {
            return resolution[1];
        }

        bvh::Ray<Scalar> pixel_to_ray(Scalar u, Scalar v) {
            // Generate rays in the camera frame:
            Vector3 dir = bvh::normalize(Vector3((-center[0]+u)/scale[0], (center[1]-v)/scale[1], -focal_length));

            // Rotate rays to the world frame (NOTE: the TRANSPOSE of the provided rotation is used for this)
            Vector3 temp;
            temp[0] = rotation[0][0]*dir[0] + rotation[1][0]*dir[1] + rotation[2][0]*dir[2];
            temp[1] = rotation[0][1]*dir[0] + rotation[1][1]*dir[1] + rotation[2][1]*dir[2];
            temp[2] = rotation[0][2]*dir[0] + rotation[1][2]*dir[1] + rotation[2][2]*dir[2];
            dir = temp;
            // std::cout << dir[0] << " " << dir[1] << " " << dir[2] << "\n";

            // Return the ray object:
            bvh::Ray<Scalar> ray(position, dir);
            return ray;
        }
};

#endif