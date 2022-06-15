#ifndef __TRACE_H
#define __TRACE_H

#include <chrono>
#include "bvh/bvh.hpp"
#include "bvh/single_ray_traverser.hpp"
#include "bvh/primitive_intersectors.hpp"
#include "bvh/triangle.hpp"

#include "lights.hpp"
#include "cameras.hpp"

template <typename Scalar>
std::vector<uint8_t> path_trace(std::unique_ptr<CameraModel<Scalar>> &camera, 
                                std::vector<std::unique_ptr<Light<Scalar>>> &lights,
                                bvh::Bvh<Scalar> &bvh_cache,
                                std::vector<bvh::Triangle<Scalar>> triangles,
                                int min_samples, int max_samples, Scalar noise_threshold, int num_bounces){

    auto start = std::chrono::high_resolution_clock::now();

    auto tri_data = triangles.data();
    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> closest_intersector(bvh_cache, tri_data);
    bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> any_int(bvh_cache, tri_data);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh_cache);

    // RBGA
    size_t width  = (size_t) floor(camera->get_resolutionX());
    size_t height = (size_t) floor(camera->get_resolutionY());
    auto pixels = std::make_unique<float[]>(4 * width * height);

    // Initialize random number generator:
    std::random_device rd;
    std::minstd_rand eng(rd());
    std::uniform_real_distribution<Scalar> distr(-0.5, 0.5);
    std::uniform_real_distribution<Scalar> dist1(0.0, 1.0);

    // Run parallel if available:
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
            // Loop through all samples for a given pixel:
            Color pixel_radiance(0);
            for (int sample = 1; sample < max_samples+1; ++sample) {
                // TODO: Make a better random sampling algorithm:
                bvh::Ray<Scalar> ray;
                auto i_rand = distr(eng);
                auto j_rand = distr(eng);
                if (max_samples == 1) {
                    ray = camera->pixel_to_ray(i, j);
                }
                else {
                    ray = camera->pixel_to_ray(i + i_rand, j + j_rand);
                }
                Color path_radiance(0);
                auto hit = traverser.traverse(ray, closest_intersector);

                // Loop through all bounces
                auto weight = Color(2*M_PI);
                int bounce = 0;
                for (; bounce < num_bounces; ++bounce){
                    if (!hit) {
                        break;
                    }
                    auto &tri = tri_data[hit->primitive_index];
                    auto u = hit->intersection.u;
                    auto v = hit->intersection.v;

                    auto normal = bvh::normalize(tri.n);
                    bvh::Vector3<Scalar> interp_normal;
                    if (tri.parent->smooth_shading){
                        interp_normal = bvh::normalize(u*tri.vn1 + v*tri.vn2 + (Scalar(1.0)-u-v)*tri.vn0);
                    }
                    else {
                        interp_normal = normal;
                    }
                    bvh::Vector<float, 2> interp_uv = (float)u*tri.uv[1] + (float)v*tri.uv[2] + (float)(Scalar(1.0)-u-v)*tri.uv[0];
                    auto material = tri.parent->get_material(interp_uv[0], interp_uv[1]);

                    //TODO: Figure out how to deal with the self-intersection stuff in a more proper way...
                    bvh::Vector3<Scalar> intersect_point = (u*tri.p1() + v*tri.p2() + (1-u-v)*tri.p0);
                    Scalar scale = 0.0001;
                    intersect_point = intersect_point - scale*normal;

                    // Calculate the direct illumination:
                    Color light_radiance(0);

                    // Loop through all provided lights:
                    for (auto& light : lights){
                        bvh::Ray<Scalar> light_ray = light->sample_ray(intersect_point);
                        Color light_color = illumination(traverser, any_int, interp_uv[0], interp_uv[1], light_ray, ray, interp_normal, material);
                        light_radiance += light_color * (float) light->get_intensity(intersect_point);
                    };

                    if (bounce >= 1) {
                        for (int idx = 0; idx < 3; ++idx){
                            light_radiance[idx] = std::clamp(light_radiance[idx], float(0), float(1));
                        }
                    }

                    // Update the path radiance with the newly calculated radiance:
                    path_radiance += light_radiance*weight;

                    // Exit if max bounces is reached:
                    if (bounce == num_bounces-1) {
                        break;
                    }

                    // Cast next ray:
                    auto [new_direction, bounce_color] = material->sample(ray, interp_normal, interp_uv[0], interp_uv[1]);
                    ray = bvh::Ray<Scalar>(intersect_point, new_direction);
                    hit = traverser.traverse(ray, closest_intersector);
                    weight *= bounce_color;
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
    std::cout << "    Path tracing completed in " << duration.count()/1000000.0 << " seconds\n\n";

    return image;
}

#endif