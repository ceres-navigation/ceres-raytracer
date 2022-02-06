#ifndef __BRDFS_H
#define __BRDFS_H

#include <bvh/bvh.hpp>

template <typename Scalar>
void lambertian(bvh::Ray<Scalar> light_ray, bvh::Vector3<Scalar> normal, bvh::Vector3<Scalar> &intensity){
    bvh::Vector3<Scalar> color(0.5,0.5,0.5);
    Scalar light_intensity = 1;
    Scalar L_dot_N = light_ray.direction[0]*normal[0] + light_ray.direction[1]*normal[1] +light_ray.direction[2]*normal[2];
    intensity[0] = L_dot_N*color[0]*light_intensity;
    intensity[1] = L_dot_N*color[1]*light_intensity;
    intensity[2] = L_dot_N*color[2]*light_intensity;
};


#endif