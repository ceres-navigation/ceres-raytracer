#ifndef __CERESRT_H
#define __CERESRT_H

// From old scene.hpp
#include "entity.hpp"

#include <bvh/binned_sah_builder.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/parallel_reinsertion_optimizer.hpp>
#include <bvh/node_layout_optimizer.hpp>

// From old render.hpp
#include <cstdint>
// #include <math.h>
#include <cmath>
#include <random>
#include <iomanip>

#include "bvh/bvh.hpp"
#include "bvh/single_ray_traverser.hpp"
#include "bvh/primitive_intersectors.hpp"
#include "bvh/triangle.hpp"

#include "lighting.hpp"
#include "cameras.hpp"

#include "materials/brdfs.hpp"



template <typename Scalar, typename Intersector>
Color illumination(bvh::SingleRayTraverser<bvh::Bvh<Scalar>> &traverser, Intersector &intersector, 
                                  float u, float v, const bvh::Ray<Scalar> &light_ray, 
                                  const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, Material<Scalar> *material) {
    Color intensity(0);
    auto hit = traverser.traverse(light_ray, intersector);
    if (!hit) {
        intensity = material->compute(light_ray, view_ray, normal, u, v);
    }
    return intensity;
}


template <typename Scalar>
std::vector<uint8_t> render(std::unique_ptr<CameraModel<Scalar>> &camera, std::vector<std::unique_ptr<Light<Scalar>>> &lights, std::vector<Entity<Scalar>*> entities,
                            int min_samples, int max_samples, Scalar noise_threshold, int num_bounces) {
    min_samples = min_samples;
    max_samples = max_samples;
    noise_threshold = noise_threshold;
    num_bounces = num_bounces;

    size_t width  = (size_t) floor(camera->get_resolutionX());
    size_t height = (size_t) floor(camera->get_resolutionY());

    // Store triangles locally:
    std::vector<bvh::Triangle<Scalar>> triangles;
    for (auto entity : entities) {
        triangles.insert(triangles.end(), entity->triangles.begin(), entity->triangles.end());
    }

    // Build an acceleration data structure for this object set
    bvh::Bvh<Scalar> bvh;

    size_t reference_count = triangles.size();
    std::unique_ptr<bvh::Triangle<Scalar>[]> shuffled_triangles;

    std::cout << "\nBuilding BVH ( using SweepSahBuilder )... for " << triangles.size() << " triangles\n";
    using namespace std::chrono;
    auto start = high_resolution_clock::now();

    auto tri_data = triangles.data();
    auto bboxes_and_centers = bvh::compute_bounding_boxes_and_centers(tri_data, triangles.size());
    auto bboxes = bboxes_and_centers.first.get(); 
    auto centers = bboxes_and_centers.second.get(); 
    
    auto global_bbox = bvh::compute_bounding_boxes_union(bboxes, triangles.size());

    bvh::SweepSahBuilder<bvh::Bvh<Scalar>> builder(bvh);
    builder.build(global_bbox, bboxes, centers, reference_count);

    bvh::ParallelReinsertionOptimizer<bvh::Bvh<Scalar>> pro_opt(bvh);
    pro_opt.optimize();

    bvh::NodeLayoutOptimizer<bvh::Bvh<Scalar>> nlo_opt(bvh);
    nlo_opt.optimize();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    std::cout << "    BVH of "
        << bvh.node_count << " node(s) and "
        << reference_count << " reference(s)\n";
    std::cout << "    BVH built in " << duration.count()/1000000.0 << " seconds\n\n";

    // RBGA
    auto pixels = std::make_unique<float[]>(4 * width * height);
    
    // Run parallel if OPENMP is available:
    #ifdef _OPENMP
        #pragma omp parallel
        {
            #pragma omp single
            std::cout << "Rendering image on " << omp_get_num_threads() << " threads..." << std::endl;
        }
    #else
        std::cout << "Rendering image on single thread..." << std::endl;
    #endif

    // Start the rendering process:
    start = high_resolution_clock::now();
    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> closest_intersector(bvh, tri_data);
    bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> any_int(bvh, tri_data);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh);

    // Initialize random number generator:
    std::random_device rd;
    std::minstd_rand eng(rd());
    std::uniform_real_distribution<Scalar> distr(-0.5, 0.5);
    std::uniform_real_distribution<Scalar> dist1(0.0, 1.0);

    #pragma omp parallel for
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

                // If no bouncesm, return just the vertex normal as the color:
                if (num_bounces == 0) {
                    if (hit) {
                        auto &tri = tri_data[hit->primitive_index];
                        auto u = hit->intersection.u;
                        auto v = hit->intersection.v;
                        auto normal = tri.parent->smooth_shading ? bvh::normalize(u*tri.vn1 + v*tri.vn2 + (Scalar(1.0)-u-v)*tri.vn0) : bvh::normalize(tri.n);
                        path_radiance[0] = std::abs(normal[0]);
                        path_radiance[1] = std::abs(normal[1]);
                        path_radiance[2] = std::abs(normal[2]);
                    }
                }

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

    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    std::cout << "    Tracing completed in " << duration.count()/1000000.0 << " seconds\n\n";


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
    return image;
};

template <typename Scalar> 
std::vector<Scalar> get_intersections(std::unique_ptr<CameraModel<Scalar>> &camera, std::vector<Entity<Scalar>*> entities){
    // Get camera resolution dimensions:
    size_t width  = (size_t) floor(camera->get_resolutionX());
    size_t height = (size_t) floor(camera->get_resolutionY());

    // Store triangles locally:
    std::vector<bvh::Triangle<Scalar>> triangles;
    for (auto entity : entities) {
        triangles.insert(triangles.end(), entity->triangles.begin(), entity->triangles.end());
    }

    // Build an acceleration data structure for this object set
    bvh::Bvh<Scalar> bvh;

    size_t reference_count = triangles.size();
    std::unique_ptr<bvh::Triangle<Scalar>[]> shuffled_triangles;

    std::cout << "\nBuilding BVH ( using SweepSahBuilder )... for " << triangles.size() << " triangles\n";
    using namespace std::chrono;
    auto start = high_resolution_clock::now();

    auto tri_data = triangles.data();
    auto bboxes_and_centers = bvh::compute_bounding_boxes_and_centers(tri_data, triangles.size());
    auto bboxes = bboxes_and_centers.first.get(); 
    auto centers = bboxes_and_centers.second.get(); 
    
    auto global_bbox = bvh::compute_bounding_boxes_union(bboxes, triangles.size());

    bvh::SweepSahBuilder<bvh::Bvh<Scalar>> builder(bvh);
    builder.build(global_bbox, bboxes, centers, reference_count);

    bvh::ParallelReinsertionOptimizer<bvh::Bvh<Scalar>> pro_opt(bvh);
    pro_opt.optimize();

    bvh::NodeLayoutOptimizer<bvh::Bvh<Scalar>> nlo_opt(bvh);
    nlo_opt.optimize();

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    std::cout << "    BVH of "
        << bvh.node_count << " node(s) and "
        << reference_count << " reference(s)\n";
    std::cout << "    BVH built in " << duration.count()/1000000.0 << " seconds\n\n";

    // Run parallel if OPENMP is available:
    #ifdef _OPENMP
        #pragma omp parallel
        {
            #pragma omp single
            std::cout << "Calculating ray intersections on " << omp_get_num_threads() << " threads..." << std::endl;
        }
    #else
        std::cout << "Calculating ray intersections on single thread..." << std::endl;
    #endif

    // Start the rendering process:
    start = high_resolution_clock::now();
    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> closest_intersector(bvh, tri_data);
    bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> any_int(bvh, tri_data);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh);

    // Define the output array:
    std::vector<Scalar> intersections;
    intersections.reserve(3*width*height);

    #pragma omp parallel for
    for(size_t i = 0; i < width; ++i) {
        for(size_t j = 0; j < height; ++j) {
            // Cast ray:
            bvh::Ray<Scalar> ray;
            ray = camera->pixel_to_ray(i, j);

            // Traverse ray through BVH:
            auto hit = traverser.traverse(ray, closest_intersector);

            // Store intersection point:
            bvh::Vector3<Scalar> intersect_point;
            if (hit) {
                auto &tri = tri_data[hit->primitive_index];
                auto u = hit->intersection.u;
                auto v = hit->intersection.v;
                intersect_point = bvh::Vector3<Scalar>(u*tri.p1() + v*tri.p2() + (1-u-v)*tri.p0);
            }
            else {
                // Zeros are fine for now, but maybe consider making these inf/nan or something?
                intersect_point = bvh::Vector3<Scalar>(0,0,0);
            }

            // Store the current intersection into the output array:
            intersections[3*width*j + 3*i + 0] = (Scalar) intersect_point[0];
            intersections[3*width*j + 3*i + 1] = (Scalar) intersect_point[1];
            intersections[3*width*j + 3*i + 2] = (Scalar) intersect_point[2];
        }
    }
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    std::cout << "    Tracing intersections completed in " << duration.count()/1000000.0 << " seconds\n\n";

    return intersections;
};

// template <typename Scalar>
// std::vector<Scalar> trace_rays(std::vector<bvh::Ray<Scalar>> rays, std::vector<Entity<Scalar>*> entities){

// };
#endif