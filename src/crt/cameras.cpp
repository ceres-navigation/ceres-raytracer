#include "crt/cameras.hpp"

template <typename Scalar>
PinholeCamera<Scalar>::PinholeCamera(CameraStruct<Scalar> camera_struct) {
    this -> position = Vector3(camera_struct.position[0],
                                camera_struct.position[1],
                                camera_struct.position[2]);

    for (int i = 0; i < 3; i++){
        for (int j = 0; j <3; j++){
            this -> rotation[i][j] = camera_struct.rotation[i][j];
        }
    }

    this -> focal_length = camera_struct.focal_length;
    this -> resolution[0] = camera_struct.resolution[0];
    this -> resolution[1] = camera_struct.resolution[1];
    this -> sensor_size[0] = camera_struct.sensor_size[0];
    this -> sensor_size[1] = camera_struct.sensor_size[1];

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

template <typename Scalar>
Scalar PinholeCamera<Scalar>::get_resolutionX() {
    return this -> resolution[0];
}

template <typename Scalar>
Scalar PinholeCamera<Scalar>::get_resolutionY() {
    return this -> resolution[1];
}

template <typename Scalar>
bvh::Ray<Scalar> PinholeCamera<Scalar>::pixel_to_ray(Scalar u, Scalar v) {
    // Generate rays in the camera frame:
    Vector3 dir = bvh::normalize(Vector3((-center[0]+u)/scale[0], (center[1]-v)/scale[1], -focal_length));

    // Rotate rays to the world frame (NOTE: the TRANSPOSE of the provided rotation is used for this)
    Vector3 temp;
    temp[0] = rotation[0][0]*dir[0] + rotation[1][0]*dir[1] + rotation[2][0]*dir[2];
    temp[1] = rotation[0][1]*dir[0] + rotation[1][1]*dir[1] + rotation[2][1]*dir[2];
    temp[2] = rotation[0][2]*dir[0] + rotation[1][2]*dir[1] + rotation[2][2]*dir[2];
    dir = temp;

    // Return the ray object:
    bvh::Ray<Scalar> ray(position, dir);
    return ray;
}

template class PinholeCamera<float>;
template class PinholeCamera<double>;