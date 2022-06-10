#ifndef __RENDER_H
#define __RENDER_H

// From old scene.hpp
#include "entity.hpp"

#include "path_trace.hpp"

#include <bvh/binned_sah_builder.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/parallel_reinsertion_optimizer.hpp>
#include <bvh/node_layout_optimizer.hpp>

// From old render.hpp
#include <cstdint>
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

    // Store triangles locally:
    std::vector<bvh::Triangle<Scalar>> triangles;
    for (auto entity : entities) {
        // Apply current entity transofmrations:
        auto entity_triangles = entity->triangles;
        resize_triangles(entity_triangles, entity->scale);
        rotate_triangles(entity_triangles, entity->rotation);
        translate_triangles(entity_triangles, entity->position);

        // Store into triangle vector:
        triangles.insert(triangles.end(), entity_triangles.begin(), entity_triangles.end());
    }

    // Build an acceleration data structure for this object set
    bvh::Bvh<Scalar> bvh;

    size_t reference_count = triangles.size();
    std::unique_ptr<bvh::Triangle<Scalar>[]> shuffled_triangles;

    std::cout << "\nBuilding BVH ( using SweepSahBuilder )... for " << triangles.size() << " triangles\n";

    auto start = std::chrono::high_resolution_clock::now();

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

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "    BVH of "
        << bvh.node_count << " node(s) and "
        << reference_count << " reference(s)\n";
    std::cout << "    BVH built in " << duration.count()/1000000.0 << " seconds\n\n";

    // Start the rendering process:
    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> closest_intersector(bvh, tri_data);
    bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> any_int(bvh, tri_data);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh);

    // Call the path tracer:
    auto image = path_trace(camera, lights, bvh, triangles, min_samples, max_samples, noise_threshold, num_bounces);

    return image;
};

template <typename Scalar> 
std::vector<Scalar> get_intersections(std::unique_ptr<CameraModel<Scalar>> &camera, std::vector<Entity<Scalar>*> entities){

    // Store triangles locally:
    std::vector<bvh::Triangle<Scalar>> triangles;
    for (auto entity : entities) {
        // Apply current entity transofmrations:
        auto entity_triangles = entity->triangles;
        resize_triangles(entity_triangles, entity->scale);
        rotate_triangles(entity_triangles, entity->rotation);
        translate_triangles(entity_triangles, entity->position);

        // Store into triangle vector:
        triangles.insert(triangles.end(), entity_triangles.begin(), entity_triangles.end());
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

    // Start the rendering process:
    start = high_resolution_clock::now();
    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> closest_intersector(bvh, tri_data);
    bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> any_int(bvh, tri_data);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh);

    // Define the output array:
    std::vector<Scalar> intersections;
    size_t width  = (size_t) floor(camera->get_resolutionX());
    size_t height = (size_t) floor(camera->get_resolutionY());
    intersections.reserve(3*width*height);

    // Run parallel if available:
    #ifdef _OPENMP
        #pragma omp parallel
        {   
            #pragma omp single
            std::cout << "Calculating intersections intersected on " << omp_get_num_threads() << " threads..." << std::endl;
        }
        #pragma omp parallel for
    #else
        std::cout << "Calculating intersections intersected on single thread..." << std::endl;
    #endif
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

template <typename Scalar>
std::vector<uint32_t> get_instances(std::unique_ptr<CameraModel<Scalar>> &camera, std::vector<Entity<Scalar>*> entities){

    // Store triangles locally:
    std::vector<bvh::Triangle<Scalar>> triangles;
    for (auto entity : entities) {
        // Apply current entity transofmrations:
        auto entity_triangles = entity->triangles;
        resize_triangles(entity_triangles, entity->scale);
        rotate_triangles(entity_triangles, entity->rotation);
        translate_triangles(entity_triangles, entity->position);

        // Store into triangle vector:
        triangles.insert(triangles.end(), entity_triangles.begin(), entity_triangles.end());
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

    // Start the rendering process:
    start = high_resolution_clock::now();
    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> closest_intersector(bvh, tri_data);
    bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> any_int(bvh, tri_data);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh);

    // Define the output array:
    std::vector<uint32_t> instances;
    size_t width  = (size_t) floor(camera->get_resolutionX());
    size_t height = (size_t) floor(camera->get_resolutionY());
    instances.reserve(width*height);

    // Run parallel if available:
    #ifdef _OPENMP
        #pragma omp parallel 
        {   
            #pragma omp single
            std::cout << "Calculating instances intersected on " << omp_get_num_threads() << " threads..." << std::endl;
        }
        #pragma omp parallel for
    #else
        std::cout << "Calculating instances intersected on single thread..." << std::endl;
    #endif
    for(size_t i = 0; i < width; ++i) {
        for(size_t j = 0; j < height; ++j) {
            // Cast ray:
            bvh::Ray<Scalar> ray;
            ray = camera->pixel_to_ray(i, j);

            // Traverse ray through BVH:
            auto hit = traverser.traverse(ray, closest_intersector);

            // Store intersection point:
            uint32_t entity_instance;
            if (hit) {
                auto &tri = tri_data[hit->primitive_index];
                entity_instance = tri.parent->id;
            }
            else {
                // Zero is fine for now....
                entity_instance = 0;
            }

            // Store the current intersection into the output array:
            instances[width*j + i + 0] = entity_instance;
        }
    }
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    std::cout << "    Tracing instance intersections completed in " << duration.count()/1000000.0 << " seconds\n\n";

    return instances;
}

#endif