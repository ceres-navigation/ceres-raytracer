#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <chrono>
#include <random>

#include "lodepng/lodepng.h"

#include "bvh/bvh.hpp"

//TODO MOVE THESE TWO INTO A SINGLE FILE IN ROOT OF CRT
#include "crt/rotations.hpp"
#include "crt/transform.hpp"

#include "crt/entity.hpp"
#include "crt/ray_tracer.hpp"
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

        // Render the scene:
        auto pixels = render<double>(config.camera, config.light, config.entity,
                                     config.min_samples, config.max_samples, config.noise_threshold, config.num_bounces);
        size_t width  = (size_t) floor(config.camera->get_resolutionX());
        size_t height = (size_t) floor(config.camera->get_resolutionY());
        unsigned error = lodepng::encode(config.output, pixels, width, height);
        if(error) {
            std::cout << "PNG error " << error << ": "<< lodepng_error_text(error) << std::endl;
        }
        }
    else {
        std::cout << "Using SINGLE precision\n\n";

        // Load the configuration file:
        auto config = SceneConfig<float>(argv[1]);

        // Render the scene:
        auto pixels = render<float>(config.camera, config.light, config.entity,
                                    config.min_samples, config.max_samples, config.noise_threshold, config.num_bounces);
        size_t width  = (size_t) floor(config.camera->get_resolutionX());
        size_t height = (size_t) floor(config.camera->get_resolutionY());
        unsigned error = lodepng::encode(config.output, pixels, width, height);
        if(error) {
            std::cout << "PNG error " << error << ": "<< lodepng_error_text(error) << std::endl;
        }
    }
    return 0;
}