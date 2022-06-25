#ifndef __RIGID_BODY_H
#define __RIGID_BODY_H

#include <bvh/bvh.hpp>

template <typename Scalar>
class RigidBody {

    using Vector3 = bvh::Vector3<Scalar>;

    public:
        Vector3 position;
        Scalar rotation[3][3];

        RigidBody(){
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

#endif