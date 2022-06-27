#ifndef __BODY_FIXED_ENTITY_H
#define __BODY_FIXED_ENTITY_H

#include "materials/material.hpp"

template <typename Scalar>
class BodyFixedEntity: public RigidBody<Scalar> {
    public:
        std::string geometry_path;
        std::string geometry_type;
        bool smooth_shading;
        Color color;

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

        void set_scale(Scalar scale){
            this -> scale = scale;
        }
};

#endif