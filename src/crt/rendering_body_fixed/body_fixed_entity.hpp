#ifndef __BODY_FIXED_ENTITY_H
#define __BODY_FIXED_ENTITY_H

#include "materials/material.hpp"

template <typename Scalar>
class BodyFixedEntity {
    public:
        std::string geometry_path;
        std::string geometry_type;
        bool smooth_shading;
        Color color;

        bvh::Vector3<Scalar> position;
        Scalar rotation[3][3];
        Scalar scale;

        BodyFixedEntity(std::string geometry_path, std::string geometry_type, bool smooth_shading, Color color){
            this->geometry_path = geometry_path;
            this->geometry_type = geometry_type;
            this->smooth_shading = smooth_shading;
            this->color = color;

            // Default values for all pose information:
            this -> scale = 1;
            this -> position = bvh::Vector3<Scalar>(0,0,0);
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

        // Pose setting methods:
        void set_scale(Scalar scale){
            this -> scale = scale;
        }
        void set_position(bvh::Vector3<Scalar> position) {
            this -> position = position;
        }
        void set_rotation(Scalar rotation[3][3]) {
            for (int i = 0; i < 3; i++){
                for (int j = 0; j <3; j++){
                    this -> rotation[i][j] = rotation[i][j];
                }
            }
        }
        void set_pose(bvh::Vector3<Scalar> position, Scalar rotation[3][3]){
            set_rotation(rotation);
            set_position(position);
        }
};

#endif