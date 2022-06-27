#ifndef __DO_LIDAR_H
#define __DO_LIDAR_H

#include <chrono>

#include "bvh/bvh.hpp"
#include "bvh/single_ray_traverser.hpp"
#include "bvh/primitive_intersectors.hpp"
#include "bvh/triangle.hpp"

#include "lidars/lidar.hpp"

template <typename Scalar>
Scalar do_lidar(std::unique_ptr<Lidar<Scalar>> &lidar,
                bvh::Bvh<Scalar> &bvh_cache,
                std::vector<bvh::Triangle<Scalar>> triangles,
                int num_rays){

    // Start time of the lidar process:
    auto start = std::chrono::high_resolution_clock::now();

    auto tri_data = triangles.data();
    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> closest_intersector(bvh_cache, tri_data);
    bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> any_int(bvh_cache, tri_data);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh_cache);

    // Define the output array:
    std::vector<bvh::Ray<Scalar>> rays = lidar->cast_rays(num_rays);
    std::vector<Scalar> distances;

    // Run parallel if available:
    int num_threads;
    #ifdef _OPENMP
        #pragma omp parallel 
        {   
            #pragma omp single
            num_threads = omp_get_num_threads();
        }
        #pragma omp parallel for
    #else
        num_threads = 1;
    #endif
    for (auto ray : rays) {
        // Traverse ray through BVH:
        auto hit = traverser.traverse(ray, closest_intersector);

        // Store intersection point:
        Scalar distance;
        if (hit) {
            auto &tri = tri_data[hit->primitive_index];
            auto u = hit->intersection.u;
            auto v = hit->intersection.v;
            auto intersect_point = bvh::Vector3<Scalar>(u*tri.p1() + v*tri.p2() + (1-u-v)*tri.p0);
            distance = bvh::length(intersect_point - lidar->position);
        }
        else {
            // Zeros are fine for now, but maybe consider making these inf/nan or something?
            distance = 0;
        }

        distances.push_back(distance);
    }

    Scalar d_sum = 0;
    int count = 0;
    for (auto d : distances){
        d_sum = d_sum + d;
        count++;
    }
    auto distance = d_sum/count;

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "    Lidar simulation completed in " << duration.count()/1000000.0 << " seconds (on " << num_threads << " threads)\n";

    return distance;
};

template <typename Scalar>
std::vector<Scalar> do_batch_lidar(std::unique_ptr<Lidar<Scalar>> &lidar,
                                   bvh::Bvh<Scalar> &bvh_cache,
                                   std::vector<bvh::Triangle<Scalar>> triangles,
                                   int num_rays){

    // Start time of the batch lidar process:
    auto start = std::chrono::high_resolution_clock::now();

    auto tri_data = triangles.data();
    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> closest_intersector(bvh_cache, tri_data);
    bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> any_int(bvh_cache, tri_data);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh_cache);

    // Define the output array:
    std::vector<std::vector<bvh::Ray<Scalar>>> batch_rays = lidar->batch_cast_rays(num_rays);

    int num_batches = (size_t) batch_rays.size();
    Scalar batch_distances_array[num_batches];

    // Run parallel if available:
    int num_threads;
    #ifdef _OPENMP
        #pragma omp parallel 
        {   
            #pragma omp single
            num_threads = omp_get_num_threads();
        }
        #pragma omp parallel for
    #else
        num_threads = 1;
    #endif
    for (int i = 0; i < num_batches; i++) {
        auto rays = batch_rays[i];
        std::vector<Scalar> distances;
        for (auto ray : rays) {
            // Traverse ray through BVH:
            auto hit = traverser.traverse(ray, closest_intersector);

            // Store intersection point:
            Scalar distance;
            if (hit) {
                auto &tri = tri_data[hit->primitive_index];
                auto u = hit->intersection.u;
                auto v = hit->intersection.v;
                auto intersect_point = bvh::Vector3<Scalar>(u*tri.p1() + v*tri.p2() + (1-u-v)*tri.p0);
                distance = bvh::length(intersect_point - lidar->batch_positions[i]);
            }
            else {
                // Zeros are fine for now, but maybe consider making these inf/nan or something?
                distance = 0;
            }

            distances.push_back(distance);
        }

        Scalar d_sum = 0;
        int count = 0;
        for (auto d : distances){
            d_sum = d_sum + d;
            count++;
        }
        Scalar distance_avg = d_sum/count;
        batch_distances_array[i] = distance_avg;
    }
    
    std::vector<Scalar> batch_distances;
    for (int i = 0; i < num_batches; i++){
        batch_distances.push_back(batch_distances_array[i]);
    }

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "    Batch lidar simulation completed in " << duration.count()/1000000.0 << " seconds (on " << num_threads << " threads)\n";

    return batch_distances;
};

#endif