#ifndef __BRDFS_H
#define __BRDFS_H

#include <bvh/bvh.hpp>

template <typename Scalar>
using Vector3 =  bvh::Vector3<Scalar>;

template <typename Vector3>
void lambertian(Vector3 sun_line, Vector3 normal, float &reflected_intensity){
    reflected_intensity = sun_line[0]*normal[0] + sun_line[1]*normal[1] + sun_line[2]*normal[2];
    return;
};


#endif