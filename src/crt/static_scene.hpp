#ifndef __SCENE_H
#define __SCENE_H

#include <filesystem>
#include <memory>
#include <vector>
#include <random>

#include "bvh/bvh.hpp"
#include "bvh/triangle.hpp"
#include "bvh/vector.hpp"

#include "model_loaders/happly.hpp"
#include "model_loaders/tiny_obj_loader.hpp"

#include "rotations.hpp"
#include "transform.hpp"

#include "obj_temp/obj.hpp"
#include "materials/material.hpp"

template <typename Scalar>
class StaticScene {
    public:
        bvh::Bvh<Scalar> bvh;
        std::vector<bvh::Triangle<Scalar>> triangles;

        int* tri_data;

        bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> closest_intersector;
        bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> any_int;
        bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser;

        // Constructor:
        StaticScene(std::vector<StaticEntity<Scalar>*> entities){
            // Store triangles locally:
            for (auto entity : entities) {
                triangles.insert(triangles.end(), entity->triangles.begin(), entity->triangles.end());
            }

            // Build an acceleration data structure for this object set
            size_t reference_count = triangles.size();
            std::unique_ptr<bvh::Triangle<Scalar>[]> shuffled_triangles;

            std::cout << "\nBuilding Static BVH ( using SweepSahBuilder )... for " << triangles.size() << " triangles\n";
            using namespace std::chrono;
            auto start = high_resolution_clock::now();

            tri_data = triangles.data();
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

            // Create the ray tracing structures:
            closest_intersector = bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false>(bvh, tri_data);
            any_int = bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false>(bvh, tri_data);
            traverser = bvh::SingleRayTraverser<bvh::Bvh<Scalar>>(bvh);
        }

        std::vector<uint8_t> render(std::unique_ptr<CameraModel<Scalar>> &camera, std::vector<std::unique_ptr<Light<Scalar>>> &lights,
                                    int min_samples, int max_samples, Scalar noise_threshold, int num_bounces){

            auto image = path_trace(camera, lights, traverser, closest_intersector, any_int, triangles, min_samples, max_samples, noise_threshold, num_bounces);

            return image;
        }
        
}