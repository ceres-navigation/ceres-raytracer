#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <chrono>
#include <random>

#include <Magick++.h> 

#include "bvh/bvh.hpp"
#include "INIReader.h"

//TODO MOVE THESE TWO INTO A SINGLE FILE IN ROOT OF CRT
#include "crt/rotations.hpp"
#include "crt/transform.hpp"

#include "crt/entity.hpp"
#include "crt/scene.hpp"
#include "crt/render.hpp"
#include "crt/lighting.hpp"

#include "model_loaders/obj.hpp"
#include "materials/material.hpp"


// Function for loading general settings:
void load_settings(INIReader reader, std::string &output, int &max_samples, int &min_samples, double &noise_threshold, int &num_bounces) {
    output = reader.Get("settings", "output", "render.png");
    max_samples = reader.GetInteger("settings","max_samples",1);
    min_samples = reader.GetInteger("settings","min_samples",1);
    noise_threshold = reader.GetReal("settings","noise_threshold",0.0);
    num_bounces = reader.GetInteger("settings","num_bounces",1);
};

bool use_double(INIReader &reader) {
   return reader.GetBoolean("settings", "use_double", true);
}

// Function to get the size:
template <typename Scalar>
void get_size(INIReader reader, const char* object_name, Scalar (&size)[2]) {
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

// Function to get the position:
template <typename Scalar>
void get_position(INIReader reader, const char* object_name, bvh::Vector3<Scalar> &position) {
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

Color get_color(INIReader &reader, const char* object_name) {

    Color color;

    std::string segment;
    std::vector<std::string> seglist;
    std::stringstream test_str;
    int idx;

    auto value_str = reader.Get(object_name, "color", "[0.5,0.5,0.5]");
    value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
    test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
    idx = 0;
    while(std::getline(test_str, segment, ',')) {
        color[idx] = std::stod(segment);
        idx = idx +1;
    }

    return color;
}

template <typename Scalar>
Scalar get_intensity(INIReader &reader, const char* object_name) {
    auto intensity = reader.GetReal(object_name, "intensity", 1);
    std::cout << "    intensity      : " << intensity << "\n";
    return intensity;
}

template <typename Scalar>
void get_scale(INIReader &reader, const char* object_name, Scalar &scale) {
    scale = reader.GetReal(object_name, "scale", 1);
    std::cout << "    scale          : " << scale << "\n";
}

// Function to get the rotation matrix:
template <typename Scalar>
void get_rotation(INIReader &reader, const char* object_name, Scalar (&rotation)[3][3]) {
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

// Function for loading camera settings:
template <typename Scalar>
std::unique_ptr<CameraModel<Scalar>> load_camera(INIReader reader) {
    std::cout << "Loading camera model...\n";

    struct CameraStruct<Scalar> camera_struct;

    // Get the camera model:
    camera_struct.name = reader.Get("camera", "type", "PinholeCamera");

    // Get the focal length:
    camera_struct.focal_length = reader.GetReal("camera", "focal_length", 0);

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
        camera_struct.resolution[idx] = std::stod(segment);
        idx = idx +1;
    }

    // Get the sensor size:
    value_str = reader.Get("camera", "sensor_size", "UNKNOWN");
    value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
    test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
    idx = 0;
    while(std::getline(test_str, segment, ',')) {
        camera_struct.sensor_size[idx] = std::stod(segment);
        idx = idx +1;
    }

    // Print if desired:
    std::cout << "  " << camera_struct.name << "\n";
    std::cout << "    focal_length   : " << camera_struct.focal_length << "\n";
    std::cout << "    resolution     : [" << camera_struct.resolution[0] << ", " << camera_struct.resolution[1] << "]\n";
    std::cout << "    sensor_size    : [" << camera_struct.sensor_size[0] << ", " << camera_struct.sensor_size[1] << "]\n";

    // Get the pose information:
    get_position<Scalar>(reader, "camera", camera_struct.position);
    get_rotation<Scalar>(reader, "camera", camera_struct.rotation);

    if (!strcmp("PinholeCamera", camera_struct.name.c_str()) ) {
        auto camera = std::make_unique<PinholeCamera<Scalar>>(camera_struct);
        return camera;
    };
    return nullptr;
};


// Function for loading objects:
template <typename Scalar>
std::vector<bvh::Triangle<Scalar>> load_objects(INIReader reader) {
    std::cout << "Loading .OBJs...\n";
    auto sections = reader.Sections();
    
    std::vector<bvh::Triangle<Scalar>> triangles;

    Scalar scale;
    Scalar rotation[3][3];
    bvh::Vector3<Scalar> position;

    std::string segment;
    std::vector<std::string> seglist;
    std::stringstream test_str;

    bool first_obj = true;
    for (auto it = sections.begin(); it != sections.end(); ++it) {
        if (!strcmp((*it).substr(0,3).c_str(), "obj")) {
            // Load the triangular mesh:
            std::string path_to_obj = reader.Get((*it), "path", "UNKNOWN");
            auto triangles_new = obj::load_from_file<Scalar>(path_to_obj);
            std::cout << "  " << (*it).substr(4) << " loaded from " << path_to_obj << "\n";

            // Load the position:
            get_scale<Scalar>(reader, (*it).c_str(), scale);
            get_position<Scalar>(reader, (*it).c_str(), position);
            get_rotation<Scalar>(reader, (*it).c_str(), rotation);

            // Apply the transformation:
            transform_triangles<Scalar>(triangles_new, rotation, position, scale);

            // Store the triangles:
            if (first_obj) {
                triangles = triangles_new;
                first_obj = false;
            }
            else {
                triangles.insert(triangles.end(), triangles_new.begin(), triangles_new.end() );
            };
        };
    };
    return triangles;
}

// Function for loading lights:
template <typename Scalar>
std::vector<PointLight<Scalar>> load_pointlights(INIReader reader) {
    // Get the sections from the INI file:
    std::cout << "Loading Point lights...\n";
    auto sections = reader.Sections();
    std::vector<PointLight<Scalar>> point_lights;
    bvh::Vector3<Scalar> position;

    for (auto it = sections.begin(); it != sections.end(); ++it) {
        if (!strcmp((*it).substr(0,3).c_str(), "lgt")) {
            std::string type = reader.Get((*it), "type", "UNKNOWN");
            if (!strcmp(type.c_str(),"PointLight")){
                std::cout << "  " << (*it).substr(4) << ":\n";
                get_position(reader, (*it).c_str(), position);
                Scalar intensity = get_intensity<Scalar>(reader, (*it).c_str());

                point_lights.push_back(PointLight<Scalar>(position, intensity));
            };
        };
    }
    return point_lights;
}

// Function for loading square lights:
template <typename Scalar>
std::vector<SquareLight<Scalar>> load_squarelights(INIReader reader) {
    // Get the sections from the INI file:
    std::cout << "Loading Square lights...\n";
    auto sections = reader.Sections();
    std::vector<SquareLight<Scalar>> square_lights;
    bvh::Vector3<Scalar> position;
    Scalar rotation[3][3];
    Scalar size[2];

    for (auto it = sections.begin(); it != sections.end(); ++it) {
        if (!strcmp((*it).substr(0,3).c_str(), "lgt")) {
            std::string type = reader.Get((*it), "type", "UNKNOWN");
            if (!strcmp(type.c_str(),"SquareLight")){
                std::cout << "  " << (*it).substr(4) << ":\n";
                get_size(reader, (*it).c_str(), size);
                get_position(reader, (*it).c_str(), position);
                get_rotation(reader, (*it).c_str(), rotation);
                Scalar intensity = get_intensity<Scalar>(reader, (*it).c_str());
                square_lights.push_back(SquareLight<Scalar>(position, rotation, size, intensity));
            }
        };
    }
    return square_lights;
}

// Function for loading objects:
template <typename Scalar>
void add_entities(INIReader &reader, Scene<Scalar> &scene) {
    std::cout << "Loading .OBJs...\n";
    auto sections = reader.Sections();

    Scalar scale;
    Scalar rotation[3][3];
    bvh::Vector3<Scalar> position;

    for (auto it = sections.begin(); it != sections.end(); ++it) {
        if (!strcmp((*it).substr(0,3).c_str(), "obj")) {
            // Load the triangular mesh:
            std::string path_to_obj = reader.Get((*it), "path", "UNKNOWN");
            std::string material_name = reader.Get((*it), "material", "");
            std::string path_to_tex = reader.Get((*it), "texture", "");
            Color c = get_color(reader, (*it).c_str());
            bool smooth = reader.GetBoolean((*it).c_str(), "smooth", true);

            // Load the position:
            get_scale<Scalar>(reader, (*it).c_str(), scale);
            get_position<Scalar>(reader, (*it).c_str(), position);
            get_rotation<Scalar>(reader, (*it).c_str(), rotation);
            
            std::shared_ptr<Material<Scalar>> material(nullptr);
            std::shared_ptr<UVMap<Color>> texture(nullptr);

            if (!path_to_tex.empty()) {
                texture = std::shared_ptr<UVMap<Color>>(new ImageUVMap(path_to_tex));
            } else { 
                texture = std::shared_ptr<UVMap<Color>>(new ConstantUVMap<Color>(c));
            }

            if (material_name == "mirror") {
                material = std::make_shared<MirrorMaterial<Scalar>>();
            } else if (material_name == "specular") {
                material = std::make_shared<TexturedBlinnPhongMaterial<Scalar>>(
                    texture, 
                    std::shared_ptr<UVMap<Color>>(new ConstantUVMap<Color>(Color(0,0.5,0.8))),
                    32
                );
            } else {
                material = std::make_shared<TexturedLambertianMaterial<Scalar>>(
                    texture
                );
            } 

            Entity<Scalar> entity(path_to_obj, material, smooth);
            std::cout << "  " << (*it).substr(4) << "( " << material_name << " ) with " << entity.get_triangles().size() << " triangles loaded from " << path_to_obj << "\n";
            scene.add_entity(entity, rotation, position, scale);
        };
    };
}

template <typename Scalar>
Scene<Scalar> construct_scene(INIReader &reader) {
    Scene<Scalar> s;

    // auto triangles = load_objects<Scalar>(reader);
    // s.add_triangles(triangles);
    add_entities(reader, s);

    auto point_lights = load_pointlights<Scalar>(reader);
    for (auto light : point_lights) {
        s.add_point_light(light);
    }

    auto square_lights = load_squarelights<Scalar>(reader);
    for (auto light : square_lights) {
        s.add_square_light(light);
    }

    return s;
}


template <typename Scalar>
void render_scene(INIReader &reader)
{
    int max_samples;
    int min_samples;
    double noise_threshold;
    int num_bounces;
    std::string output;

    load_settings(reader, output, max_samples, min_samples, noise_threshold, num_bounces);
    auto camera = load_camera<Scalar>(reader);
    auto scene = construct_scene<Scalar>(reader);

    scene.max_samples = max_samples;
    scene.min_samples = min_samples;
    scene.noise_threshold = Scalar(noise_threshold);
    scene.num_bounces = num_bounces;

    scene.render(*camera, output);
}

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "An INI configuration file must be provided\n";
        return 1;
    }
    else {
        std::cout << "Processing INI configuration given by: " << argv[1] << "\n";
    }

    // Parse the INI configuration file:
    INIReader reader(argv[1]);

    if (use_double(reader)) {
        // Build and render the scene as double precision:
        render_scene<double>(reader);
    } else {
        // Build and render the scene as single precision:
        render_scene<float>(reader);
    }
    return 0;
}