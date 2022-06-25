#ifndef __DO_RENDER_H
#define __DO_RENDER_H

// From old scene.hpp
#include <chrono>
#include <bvh/binned_sah_builder.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/parallel_reinsertion_optimizer.hpp>
#include <bvh/node_layout_optimizer.hpp>

// From old render.hpp
#include <cstdint>
#include <cmath>
#include <iomanip>

#include "bvh/bvh.hpp"
#include "bvh/single_ray_traverser.hpp"
#include "bvh/primitive_intersectors.hpp"
#include "bvh/triangle.hpp"

#include "cameras/camera.hpp"
#include "lights/light.hpp"
#include "materials/brdfs.hpp"
#include "rendering_dynamic/entity.hpp"
#include "path_tracing/unidirectional.hpp"

template <typename Scalar>
std::vector<uint8_t> do_render(std::unique_ptr<Camera<Scalar>> &camera, 
                               std::vector<std::unique_ptr<Light<Scalar>>> &lights, 
                               bvh::Bvh<Scalar> &bvh,
                               std::vector<bvh::Triangle<Scalar>> &triangles,
                               int min_samples, int max_samples, Scalar noise_threshold, int num_bounces) {

    // Start time of the rendering process:
    auto start = std::chrono::high_resolution_clock::now();

    auto tri_data = triangles.data();

    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> closest_intersector(bvh, tri_data);
    bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> any_intersector(bvh, tri_data);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh);

    // RBGA
    size_t width  = (size_t) floor(camera->get_resolutionX());
    size_t height = (size_t) floor(camera->get_resolutionY());
    auto pixels = std::make_unique<float[]>(4 * width * height);

    // Initialize random number generator:
    std::random_device rd;
    std::minstd_rand eng(rd());
    std::uniform_real_distribution<Scalar> distr(-0.5, 0.5);
    std::uniform_real_distribution<Scalar> dist1(0.0, 1.0);

    // Default for now:
    std::string path_tracing_type = "unidirectional";

    // Sample pixels:
    #ifdef _OPENMP
        #pragma omp parallel 
        {   
            #pragma omp single
            std::cout << "Path tracing on " << omp_get_num_threads() << " threads..." << std::endl;
        }
        #pragma omp parallel for
    #else
        std::cout << "Path tracing on single thread..." << std::endl;
    #endif
    for(size_t i = 0; i < width; ++i) {
        for(size_t j = 0; j < height; ++j) {
            size_t index = 4 * (width * j + i);
            Color pixel_radiance(0);
            
            for (int sample = 1; sample < max_samples+1; ++sample) {

                // Generate a random sample:
                bvh::Ray<Scalar> ray;
                auto i_rand = distr(eng);
                auto j_rand = distr(eng);
                if (max_samples == 1) {
                    ray = camera->pixel_to_ray(i, j);
                }
                else {
                    ray = camera->pixel_to_ray(i + i_rand, j + j_rand);
                }

                // Perform path tracing operation:
                Color path_radiance(0);
                if (path_tracing_type.compare("unidirectional") == 0) {
                    path_radiance = unidirectional(lights, tri_data, closest_intersector, 
                                                   any_intersector, traverser, ray, num_bounces);
                }
                else if (path_tracing_type.compare("bidirectional") == 0){
                    // NOT YET IMPLEMENTED
                }
                else if (path_tracing_type.compare("metropolis") == 0){
                    // NOT YET IMPLEMENTED
                }

                // Run adaptive sampling:
                auto rad_contrib = (path_radiance - pixel_radiance)*(1.0f/sample);
                pixel_radiance += rad_contrib;
                if (sample >= min_samples) {
                    Scalar noise = bvh::length(rad_contrib);
                    if (noise < noise_threshold) {
                        break;
                    }
                }
            }
            // Store the pixel intensity:
            pixels[index    ] = pixel_radiance[0];
            pixels[index + 1] = pixel_radiance[1];
            pixels[index + 2] = pixel_radiance[2];
            pixels[index + 3] = 1;
        }
    }

    // Construct output image:
    std::vector<uint8_t> image;
    image.reserve(4*width*height);

    for (size_t j = 0; j < 4*width*height; j++) {
        image.push_back((uint8_t) std::clamp(pixels[j] * 256, 0.0f, 255.0f));
    }

    for(unsigned y = 0; y < height; y++) {
        for(unsigned x = 0; x < width; x++) {
            size_t i = 4 * (width * y + x);
            image[4 * width * y + 4 * x + 0] = (uint8_t) std::clamp(pixels[i+0] * 256, 0.0f, 255.0f);
            image[4 * width * y + 4 * x + 1] = (uint8_t) std::clamp(pixels[i+1] * 256, 0.0f, 255.0f);
            image[4 * width * y + 4 * x + 2] = (uint8_t) std::clamp(pixels[i+2] * 256, 0.0f, 255.0f);
            image[4 * width * y + 4 * x + 3] = (uint8_t) std::clamp(pixels[i+3] * 256, 0.0f, 255.0f);
        }
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "    Rendering completed in " << duration.count()/1000000.0 << " seconds\n\n";

    return image;
};

#endif