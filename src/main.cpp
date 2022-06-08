#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <chrono>
#include <random>

#include <Magick++.h> 

#include "bvh/bvh.hpp"

//TODO MOVE THESE TWO INTO A SINGLE FILE IN ROOT OF CRT
#include "crt/rotations.hpp"
#include "crt/transform.hpp"

#include "crt/entity.hpp"
#include "crt/scene.hpp"
#include "crt/render.hpp"
#include "crt/lighting.hpp"

#include "materials/material.hpp"

#include "crt/sceneconfig.hpp"

 //TODO REMOVE THIS:
 #include "obj_temp/obj.hpp"


int main(int argc, char** argv) {

    // Parse the INI configuration file:
    if (argc < 2) {
        std::cout << "An INI configuration file must be provided\n";
        return 1;
    }
    std::cout << "Processing INI configuration given by: " << argv[1] << "\n";

    bool use_double = false;
    if (argc > 2){
        if (strcmp(argv[2], "-d") == 0) {
            use_double = true;
        }
    }

    if (use_double) {
        std::cout << "Using DOUBLE precision\n\n";

        // Load the configuration file:
        auto config = SceneConfig<double>(argv[1]);

        // Create and configure the scene:
        Scene<double> scene(config.min_samples, config.max_samples, config.noise_threshold, config.num_bounces);
        scene.add_camera(config.camera);
        for (auto& light : config.lights){
            scene.add_light(light);
        }
        for (Entity<double>* entity : config.entities) {
            for (auto &tri : entity->triangles) {
                std::cout << tri.parent->smooth_shading << "\n";
                break;
            }
            scene.add_entity(entity);
        }

        // Render the scene:
        scene.render(config.output);
    }
    else {
        std::cout << "Using SINGLE precision\n\n";

        // Load the configuration file:
        auto config = SceneConfig<float>(argv[1]);

        // Create and configure the scene:
        Scene<float> scene(config.min_samples, config.max_samples, config.noise_threshold, config.num_bounces);
        scene.add_camera(config.camera);
        for (auto& light : config.lights){
            scene.add_light(light);
        }
        for (Entity<float>* entity : config.entities) {
            scene.add_entity(entity);
        }

        // Render the scene:
        scene.render(config.output);
    }
    return 0;
}