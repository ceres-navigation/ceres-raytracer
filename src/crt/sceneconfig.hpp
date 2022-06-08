#ifndef __SCENECONFIG_H
#define __SCENECONFIG_H


#include <vector>
#include <iostream>

#include "INIReader/INIReader.hpp"


template <typename Scalar>
class SceneConfig {
    public:
        std::string output;
        
        int min_samples;
        int max_samples;
        Scalar noise_threshold;
        int num_bounces;

        std::unique_ptr<CameraModel<Scalar>> camera;
        std::unique_ptr<Light<Scalar>> light;
        Entity<Scalar>* entity;

        SceneConfig(std::string config_file){
            INIReader reader(config_file);
            // Load the settings:
            this->output = reader.Get("settings", "output", "render.png");
            this->max_samples = reader.GetInteger("settings","max_samples",1);
            this->min_samples = reader.GetInteger("settings","min_samples",1);
            this->noise_threshold = reader.GetReal("settings","noise_threshold",0.0);
            this->num_bounces = reader.GetInteger("settings","num_bounces",1);

            // Load the camera:
            this->camera = std::move(load_camera(reader));

            // Load the lights:
            this->light = load_light(reader);

            // Load the entities:
            this->entity = load_entity(reader);
        }

        // Static method for loading the camera model:
        static std::unique_ptr<CameraModel<Scalar>> load_camera(INIReader &reader) {
            std::cout << "Loading camera model...\n";

            bvh::Vector3<Scalar> position;
            Scalar rotation[3][3];
            Scalar focal_length;
            Scalar resolution[2];
            Scalar sensor_size[2];

            // Get the camera model:
            std::string name = reader.Get("camera", "type", "PinholeCamera");

            // Get the focal length:
            focal_length = reader.GetReal("camera", "focal_length", 0);

            // Declare intermediate variables for parsing arrays:
            std::string segment;
            std::vector<std::string> seglist;
            std::stringstream test_str;
            int idx;

            // Get the resolution:
            auto value_str = reader.Get("camera", "resolution", "UNKNOWN");
            value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
            test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
            idx = 0;
            while(std::getline(test_str, segment, ',')) {
                resolution[idx] = std::stod(segment);
                idx = idx +1;
            }

            // Get the sensor size:
            value_str = reader.Get("camera", "sensor_size", "UNKNOWN");
            value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
            test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
            idx = 0;
            while(std::getline(test_str, segment, ',')) {
                sensor_size[idx] = std::stod(segment);
                idx = idx +1;
            }

            // Print if desired:
            std::cout << "  " << name << "\n";
            std::cout << "    focal_length   : " << focal_length << "\n";
            std::cout << "    resolution     : [" << resolution[0] << ", " << resolution[1] << "]\n";
            std::cout << "    sensor_size    : [" << sensor_size[0] << ", " << sensor_size[1] << "]\n";

            // Get the pose information:
            get_position(reader, "camera", position);
            get_rotation(reader, "camera", rotation);

            if (!strcmp("PinholeCamera", name.c_str()) ) {
                auto camera = std::make_unique<PinholeCamera<Scalar>>(focal_length, resolution, sensor_size);
                camera->set_pose(position, rotation);
                return camera;
            };
            return nullptr;
        }

        // Static method for loading lights:
        static std::unique_ptr<Light<Scalar>> load_light(INIReader &reader) {
            // Get the sections:
            auto sections = reader.Sections();

            // Create the vector:
            bvh::Vector3<Scalar> position;
            Scalar rotation[3][3];
            Scalar size[2];

            std::cout << "Loading lights...\n";
            for (auto it = sections.begin(); it != sections.end(); ++it) {
                if (!strcmp((*it).substr(0,3).c_str(), "lgt")) {
                    std::string type = reader.Get((*it), "type", "UNKNOWN");

                    // Load the point lights:
                    if (!strcmp(type.c_str(),"PointLight")){
                        std::cout << "  " << type.c_str() << ":\n";
                        get_position(reader, (*it).c_str(), position);
                        Scalar intensity = get_intensity(reader, (*it).c_str());
                        auto light = new PointLight<Scalar>(intensity);
                        light->set_position(position);
                        return std::unique_ptr<Light<Scalar>>(light);
                    }

                    // Load the square lights:
                    else if (!strcmp(type.c_str(),"SquareLight")){
                        std::cout << "  " << type.c_str() << ":\n";
                        Scalar intensity = get_intensity(reader, (*it).c_str());
                        get_size(reader, (*it).c_str(), size);
                        get_position(reader, (*it).c_str(), position);
                        get_rotation(reader, (*it).c_str(), rotation);
                        auto light = new SquareLight<Scalar>(size, intensity);
                        light->set_position(position);
                        light->set_rotation(rotation);
                        return std::unique_ptr<Light<Scalar>>(light);
                    }
                }
            }
            return nullptr;
        }

        // Static method for loading entities:
        static Entity<Scalar>* load_entity(INIReader &reader) {
            std::cout << "Loading 3d-models...\n";

            auto sections = reader.Sections();

            Scalar scale;
            bvh::Vector3<Scalar> position;
            Scalar rotation[3][3];
            Color color;

            for (auto it = sections.begin(); it != sections.end(); ++it) {
                if (!strcmp((*it).substr(0,3).c_str(), "obj")) {
                    // Get the configurations for the model:
                    std::string path_to_model = reader.Get((*it), "path", "UNKNOWN");
                    bool smooth_shading = reader.GetBoolean((*it).c_str(), "smooth", true);
                    get_scale(reader, (*it).c_str(), scale);
                    get_position(reader, (*it).c_str(), position);
                    get_rotation(reader, (*it).c_str(), rotation);
                    get_color(reader, (*it).c_str(), color);

                    // Create the new entity instance:
                    Entity<Scalar>* new_entity = new Entity<Scalar>(path_to_model, smooth_shading, color);
                    new_entity->set_scale(scale);
                    new_entity->set_position(position);
                    new_entity->set_rotation(rotation);

                    for (auto &tri : new_entity->triangles) { 
                        std::cout << "INSIDE SCENECONFIG:" << tri.parent -> smooth_shading << "\n";
                        break;
                    }

                    // Insert to vector of entities:
                    // entities.emplace_back(new_entity);
                    return new_entity;
                }
            }

            return nullptr;
        }


    private:
        // Static method to get the size:
        static void get_size(INIReader reader, const char* object_name, Scalar (&size)[2]) {
            std::string segment;
            std::vector<std::string> seglist;
            std::stringstream test_str;
            int idx;

            auto value_str = reader.Get(object_name, "size", "[1,1]");
            value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
            test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
            idx = 0;
            while(std::getline(test_str, segment, ',')) {
                size[idx] = std::stod(segment);
                idx = idx +1;
            }
            std::cout << "    size           : [" << size[0] << ", " << size[1] << "]\n";
        }

        // Static method to get the position:
        static void get_position(INIReader reader, const char* object_name, bvh::Vector3<Scalar> &position) {
            std::string segment;
            std::vector<std::string> seglist;
            std::stringstream test_str;
            int idx;

            auto value_str = reader.Get(object_name, "position", "[0,0,0]");
            value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
            test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
            idx = 0;
            while(std::getline(test_str, segment, ',')) {
                position[idx] = std::stod(segment);
                idx = idx +1;
            }
            std::cout << "    position       : [" << position[0] << ", " << position[1] << ", " << position[2] << "]\n";
        }

        // Static method to get the intensity
        static Scalar get_intensity(INIReader &reader, const char* object_name) {
            auto intensity = reader.GetReal(object_name, "intensity", 1);
            std::cout << "    intensity      : " << intensity << "\n";
            return intensity;
        }

        // Static method to get the scale
        static void get_scale(INIReader &reader, const char* object_name, Scalar &scale) {
            scale = reader.GetReal(object_name, "scale", 1);
            std::cout << "    scale          : " << scale << "\n";
        }

        // Static method to get the color
        static void get_color(INIReader &reader, const char* object_name, Color &color) {
            Scalar color_code[3];
            std::string segment;
            auto value_str = reader.Get(object_name, "color", "UNKNOWN");
            value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
            auto test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
            int idx = 0;
            while(std::getline(test_str, segment, ',')) {
                color_code[idx] = std::stod(segment);
                idx = idx +1;
            }
            color = Color(color_code[0], color_code[1], color_code[2]);
        }

        // Static method to get the rotation matrix:
        static void get_rotation(INIReader &reader, const char* object_name, Scalar (&rotation)[3][3]) {
            std::string segment;
            std::vector<std::string> seglist;
            std::stringstream test_str;
            int idx;

            // Get the quaternion:
            auto value_str = reader.Get(object_name, "quaternion", "[0,0,0,1]");
            value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
            test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")-1));
            idx = 0;
            Scalar quaternion[4];
            while(std::getline(test_str, segment, ',')) {
                quaternion[idx] = std::stod(segment);
                idx = idx +1;
            }
            quaternion[0] = 0.0;
            quaternion[1] = 0.0;
            quaternion[2] = 0.0;
            quaternion[3] = 1.0;

            std::string euler_sequence;
            Scalar euler_angles[3];

            // Determine the rotation matrix:
            value_str = reader.Get(object_name, "euler_angles", "UNKNOWN");
            if (strcmp(value_str.c_str(),"UNKNOWN")){
                // If any euler angles are provided, use them:
                euler_sequence = reader.Get(object_name, "euler_sequence", "321");
                value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
                test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
                idx = 0;
                while(std::getline(test_str, segment, ',')) {
                    euler_angles[idx] = std::stod(segment);
                    idx = idx +1;
                }
                euler_to_rotation<Scalar>(euler_angles, euler_sequence.c_str(), rotation);
            }
            else {
                // If no euler angles provided, default to the quaternion:
                quaternion_to_rotation<Scalar>(quaternion, rotation);
            }

            if (strcmp(value_str.c_str(),"UNKNOWN")) {
                std::cout << "    euler_sequence : " << euler_sequence[0] << "-" << euler_sequence[1] << "-" << euler_sequence[2] <<"\n";
                std::cout << "    euler_angles   : [" << euler_angles[0] << ", " << euler_angles[1] << ", " << euler_angles[2]  <<"]\n\n";
            }
            else {
                std::cout << "    quaternion     : [" << quaternion[0] << ", " << quaternion[1] << ", " << quaternion[2] << ", " << quaternion[3] <<"]\n\n";
            }
        }

};

#endif