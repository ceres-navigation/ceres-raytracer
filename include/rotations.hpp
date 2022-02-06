#ifndef __ROTATIONS_H
#define __ROTATIONS_H

template <typename Scalar>
void quaternion_to_rotation(Scalar quaternion[4], Scalar (&rotation)[3][3]) {
    // NOTE: input quaternions are interpreted as shuster notation.
    rotation[0][0] =   (quaternion[0]*quaternion[0]) - (quaternion[1]*quaternion[1]) - (quaternion[2]*quaternion[2]) + (quaternion[3]*quaternion[3]);
    rotation[0][1] = 2*(quaternion[0]*quaternion[1]  +  quaternion[2]*quaternion[3]);
    rotation[0][2] = 2*(quaternion[0]*quaternion[2]  -  quaternion[1]*quaternion[3]);
    rotation[1][0] = 2*(quaternion[0]*quaternion[1]  -  quaternion[2]*quaternion[3]);
    rotation[1][1] =  -(quaternion[0]*quaternion[0]) + (quaternion[1]*quaternion[1]) - (quaternion[2]*quaternion[2]) + (quaternion[3]*quaternion[3]);
    rotation[1][2] = 2*(quaternion[1]*quaternion[2]  +  quaternion[0]*quaternion[3]);
    rotation[2][0] = 2*(quaternion[0]*quaternion[2]  +  quaternion[1]*quaternion[3]);
    rotation[2][1] = 2*(quaternion[1]*quaternion[2]  -  quaternion[0]*quaternion[3]);
    rotation[2][2] =  -(quaternion[0]*quaternion[0]) - (quaternion[1]*quaternion[1]) + (quaternion[2]*quaternion[2]) + (quaternion[3]*quaternion[3]);
}

template <typename Scalar>
void MatrixMultiply(Scalar A[3][3], Scalar (&B)[3][3]) {
    Scalar temp[3][3] = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                temp[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            B[i][j] = temp[i][j];
        }
    }
}

// Define the rotation around the 3rd (z) axis:
template <typename Scalar>
void rotmat_1(Scalar angle, Scalar (&rotation_1)[3][3]){
    static constexpr Scalar pi = Scalar(3.14159265359);
    Scalar cos = std::cos(angle*pi / Scalar(180));
    Scalar sin = std::sin(angle*pi / Scalar(180));
    rotation_1[0][0] = 1;
    rotation_1[0][1] = 0;
    rotation_1[0][2] = 0;
    rotation_1[1][0] = 0;
    rotation_1[1][1] = cos;
    rotation_1[1][2] = sin;
    rotation_1[2][0] = 0;
    rotation_1[2][1] = -sin;
    rotation_1[2][2] =  cos;
}

// Define the rotation around the 2nd (y) axis:
template <typename Scalar>
void rotmat_2(Scalar angle, Scalar (&rotation_2)[3][3]){
    static constexpr Scalar pi = Scalar(3.14159265359);
    Scalar cos = std::cos(angle*pi / Scalar(180));
    Scalar sin = std::sin(angle*pi / Scalar(180));
    rotation_2[0][0] = cos;
    rotation_2[0][1] = 0;
    rotation_2[0][2] = -sin;
    rotation_2[1][0] = 0;
    rotation_2[1][1] = 1;
    rotation_2[1][2] = 0;
    rotation_2[2][0] = sin;
    rotation_2[2][1] = 0;
    rotation_2[2][2] = cos;
}

// Define the rotation around the 1st (z) axis:
template <typename Scalar>
void rotmat_3(Scalar angle, Scalar (&rotation_3)[3][3]){
    static constexpr Scalar pi = Scalar(3.14159265359);
    Scalar cos = std::cos(angle*pi / Scalar(180));
    Scalar sin = std::sin(angle*pi / Scalar(180));
    rotation_3[0][0] = cos;
    rotation_3[0][1] = sin;
    rotation_3[0][2] = 0;
    rotation_3[1][0] = -sin;
    rotation_3[1][1] =  cos;
    rotation_3[1][2] = 0;
    rotation_3[2][0] = 0;
    rotation_3[2][1] = 0;
    rotation_3[2][2] = 1;
}

template <typename Scalar>
void euler_to_rotation(Scalar euler_angles[3], const char* euler_sequence, Scalar (&rotation)[3][3]) {
    Scalar rotation_1[3][3];
    Scalar rotation_2[3][3];
    Scalar rotation_3[3][3];

    // Define an initial rotation:
    rotation[0][0] = 1;
    rotation[0][1] = 0;
    rotation[0][2] = 0;
    rotation[1][0] = 0;
    rotation[1][1] = 1;
    rotation[1][2] = 0;
    rotation[2][0] = 0;
    rotation[2][1] = 0;
    rotation[2][2] = 1;

    // Obtain the first rotation:
    if (euler_sequence[0] == '1') {
        rotmat_1(euler_angles[0], rotation_1);
    }
    else if (euler_sequence[0] == '2') {
        rotmat_2(euler_angles[0], rotation_1);
    }
    else {
        rotmat_3(euler_angles[0], rotation_1);
    }

    // Obtain the second rotaiton:
    if (euler_sequence[1] == '1') {
        rotmat_1(euler_angles[1], rotation_2);
    }
    else if (euler_sequence[1] == '2') {
        rotmat_2(euler_angles[1], rotation_2);
    }
    else {
        rotmat_3(euler_angles[1], rotation_2);
    }

    // Obtain the third rotaiton:
    if (euler_sequence[2] == '1') {
        rotmat_1(euler_angles[2], rotation_3);
    }
    else if (euler_sequence[2] == '2') {
        rotmat_2(euler_angles[2], rotation_3);
    }
    else {
        rotmat_3(euler_angles[2], rotation_3);
    }

    // Apply all three rotations to the default:
    MatrixMultiply<Scalar>(rotation_3, rotation);
    MatrixMultiply<Scalar>(rotation_2, rotation);
    MatrixMultiply<Scalar>(rotation_1, rotation);
}

#endif