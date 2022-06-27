#ifndef __PINHOLE_CAMERA_H
#define __PINHOLE_CAMERA_H

#include <bvh/bvh.hpp>

template <typename Scalar>
class SimpleCamera: public Camera<Scalar> {
    using Vector3 =  bvh::Vector3<Scalar>;
    public:
        // Specific properties for pinhole camera projection:       
        Scalar center[2];
        Scalar scale[2];

        SimpleCamera(Scalar focal_length, Scalar resolution[2], Scalar sensor_size[2], bool z_positive) {
            this -> focal_length = focal_length;
            this -> z_positive   = z_positive;

            // TODO: move to Sensor...
            this -> resolution[0] = resolution[0];
            this -> resolution[1] = resolution[1];
            this -> sensor_size[0] = sensor_size[0];
            this -> sensor_size[1] = sensor_size[1];
            this -> center[0] = resolution[0]/2.0;
            this -> center[1] = resolution[1]/2.0;
            this -> scale[0] = resolution[0]/sensor_size[0];
            this -> scale[1] = resolution[1]/sensor_size[1];
        }

        // Copy constructor:
        SimpleCamera(const SimpleCamera<Scalar> &original) : Camera<Scalar>(original) {
            this -> z_positive = original.z_positive;
            this -> focal_length = original.focal_length;

            //TODO: move to sensor copy:
            this -> resolution[0]   = original.resolution[0];
            this -> resolution[1]   = original.resolution[1];
            this -> sensor_size[0]  = original.sensor_size[0];
            this -> sensor_size[1]  = original.sensor_size[1];
            this -> center[0] = original.center[0];
            this -> center[1] = original.center[1];
            this -> scale[0]  = original.scale[0];
            this -> scale[1]  = original.scale[1];

            //TODO: move to RigidBody copy:
            this -> position = original.position;
            for (auto i = 0; i < 3; i++){
                for (auto j = 0; j < 3; j++) {
                    this->rotation[i][j] = original.rotation[i][j];
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