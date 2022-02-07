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
bvh::Ray<Scalar> cosine_importance(bvh::Vector3<Scalar> origin, bvh::Vector3<Scalar> normal, Scalar r1, Scalar r2) {
    // This implementation is based on Malley's method:

    // Generate a random sample:
    Scalar r = std::sqrt(r1);
    Scalar theta = r2*2*3.1415926;
    Scalar x = r*std::cos(theta);
    Scalar y = r*std::sin(theta);
    Scalar z = std::sqrt(std::max(0.0, 1.0 - r1*r1 - r2*r2));

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

    bvh::Vector3<Scalar> dir(x*Ny[0] + y*normal[0] + z*Nx[0], 
                             x*Ny[1] + y*normal[1] + z*Nx[1], 
                             x*Ny[2] + y*normal[2] + z*Nx[2]);

    return bvh::Ray<Scalar>(origin, bvh::normalize(dir));
}

#endif