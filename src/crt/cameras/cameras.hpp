#ifndef __CAMERAS_H
#define __CAMERAS_H

#include <bvh/bvh.hpp>

template <typename Scalar>
class CameraModel {
    using Vector3 =  bvh::Vector3<Scalar>;
    public:
        bool z_positive;

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

        void set_position(Vector3 new_position) {
            this -> position = new_position;
        }

        void set_rotation(Scalar new_rotation[3][3]) {
            for (int i = 0; i < 3; i++){
                for (int j = 0; j <3; j++){
                    this -> rotation[i][j] = new_rotation[i][j];
                }
            }
        }

        void set_pose(Vector3 new_position, Scalar new_rotation[3][3]){
            this -> position = new_position;
            for (int i = 0; i < 3; i++){
                for (int j = 0; j <3; j++){
                    this -> rotation[i][j] = new_rotation[i][j];
                }
            }
        }
};

template <typename Scalar>
class PinholeCamera: public CameraModel<Scalar> {
    using Vector3 =  bvh::Vector3<Scalar>;
    public:
        // Specific properties for pinhole camera projection:       
        Scalar center[2];
        Scalar scale[2];
        Scalar K[3][3];

        PinholeCamera(Scalar focal_length, Scalar resolution[2], Scalar sensor_size[2], bool z_positive) {
            this -> focal_length = focal_length;
            this -> resolution[0] = resolution[0];
            this -> resolution[1] = resolution[1];
            this -> sensor_size[0] = sensor_size[0];
            this -> sensor_size[1] = sensor_size[1];
            this -> z_positive = z_positive;

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

            // Default pose information:
            this -> position = Vector3(0,0,0);
            this -> rotation[0][0] = 1;
            this -> rotation[0][1] = 0;
            this -> rotation[0][2] = 0;
            this -> rotation[1][0] = 0;
            this -> rotation[1][1] = 1;
            this -> rotation[1][2] = 0;
            this -> rotation[2][0] = 0;
            this -> rotation[2][1] = 0;
            this -> rotation[2][2] = 1;
        }

        // Copy constructor:
        PinholeCamera(const PinholeCamera<Scalar> &rhs) {
            this -> z_positive = rhs.z_positive;
            this -> focal_length = rhs.focal_length;
            this -> resolution[0]   = rhs.resolution[0];
            this -> resolution[1]   = rhs.resolution[1];
            this -> sensor_size[0]  = rhs.sensor_size[0];
            this -> sensor_size[1]  = rhs.sensor_size[1];
            this -> center[0] = rhs.center[0];
            this -> center[1] = rhs.center[1];
            this -> scale[0]  = rhs.scale[0];
            this -> scale[1]  = rhs.scale[1];
            this -> position = rhs.position;
            for (auto i = 0; i < 3; i++){
                for (auto j = 0; j < 3; j++) {
                    this->K[i][j] = rhs.K[i][j];
                }
            }
            for (auto i = 0; i < 3; i++){
                for (auto j = 0; j < 3; j++) {
                    this->rotation[i][j] = rhs.rotation[i][j];
                }
            }
        }

        Scalar get_resolutionX() {
            return this->resolution[0];
        }

        Scalar get_resolutionY() {
            return this->resolution[1];
        }

        bvh::Ray<Scalar> pixel_to_ray(Scalar u, Scalar v) {
            // Generate rays in the camera frame:
            Vector3 dir;
            if (this->z_positive){
                dir = bvh::normalize(Vector3((-center[0]+u)/scale[0], (center[1]-v)/scale[1], this->focal_length));
            }
            else {
                dir = bvh::normalize(Vector3((-center[0]+u)/scale[0], (center[1]-v)/scale[1], -this->focal_length));
            } 

            // Rotate rays to the world frame (NOTE: the TRANSPOSE of the provided rotation is used for this)
            Vector3 temp;
            temp[0] = this->rotation[0][0]*dir[0] + this->rotation[1][0]*dir[1] + this->rotation[2][0]*dir[2];
            temp[1] = this->rotation[0][1]*dir[0] + this->rotation[1][1]*dir[1] + this->rotation[2][1]*dir[2];
            temp[2] = this->rotation[0][2]*dir[0] + this->rotation[1][2]*dir[1] + this->rotation[2][2]*dir[2];
            dir = temp;

            // Return the ray object:
            bvh::Ray<Scalar> ray(this->position, dir);
            return ray;
        }
};

#endif