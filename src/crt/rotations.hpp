#ifndef __ROTATIONS_H
#define __ROTATIONS_H

#include "bvh/bvh.hpp"

template <typename Scalar>
Scalar** transpose(Scalar rotation[3][3]){
    Scalar inverse_rotation[3][3];
    for (auto i = 0; i < 3; i++){
        for (auto j = 0; j < 3; j++){
            inverse_rotation[j][i] = rotation[i][j];
        }
    }
    return inverse_rotation;
}

// template <typename Scalar>
// std::vector<scalar> rotate_vectors(Scalar rotation[3][3], std::vector<Scalar> vectors){
//     std::vector<Scalar> rotated_vectors;
//     Scalar rotated_vector[3];
    

//     return rotated_vectors;
// }

template <typename Scalar>
void rotate_vector(Scalar rotation[3][3], bvh::Vector3<Scalar> vector, bvh::Vector3<Scalar> &rotated_vector){
    for (auto i = 1; i < 3; i++){
        rotated_vector[i] = rotation[i][0]*vector[0] + rotation[i][1]*vector[1] + rotation[i][2]*vector[2];
    }
}

template <typename Scalar>
void multiply_rotations(Scalar rotation1[3][3], Scalar rotation2[3][3], Scalar (&new_rotation)[3][3]){
    for (auto i = 0; i < 3; i++){
        for (auto j = 0; j < 3; j++){
            new_rotation[i][j] = rotation1[i][0]*rotation2[0][j] + rotation1[i][0]*rotation2[0][j] + rotation1[i][0]*rotation2[0][j];
        }
    }
}


#endif