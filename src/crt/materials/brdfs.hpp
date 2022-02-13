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


template <typename Scalar>
inline bvh::Vector3<Scalar> cosine_importance(bvh::Vector3<Scalar> normal, Scalar r1, Scalar r2) {
    // This implementation is based on Malley's method:
    // NOTE: Assumes left-handed normal!!!
    // Generate a random sample:
    Scalar r = std::sqrt(r1);
    Scalar theta = r2*2*3.1415926;
    Scalar x = -r*std::cos(theta);
    Scalar y = -r*std::sin(theta);
    Scalar z = -std::sqrt(std::max(0.0, 1.0 - x*x - y*y));

    // Transform back into world coordinates:
    bvh::Vector3<Scalar> Nx;
    // Per PBR, need handle two cases
    // not sure what they are, but avoids div by zero
    if (std::abs(normal[0]) > std::abs(normal[1])) {
        Nx = bvh::Vector3<Scalar>(normal[2],0.0,-normal[0])*Scalar(1.0/std::sqrt(normal[0]*normal[0] + normal[2]*normal[2]));
    } else {
        Nx = bvh::Vector3<Scalar>(0, normal[2],-normal[1])*Scalar(1.0/std::sqrt(normal[1]*normal[1] + normal[2]*normal[2]));
    }

    bvh::Vector3<Scalar> Ny = cross(normal, Nx);

    auto dir = Nx*x + Ny*y + normal*z;
    return dir;
}

#endif