#ifndef __BODY_FIXED_GROUP_H
#define __BODY_FIXED_GROUP_H

#include <memory>
#include <vector>
#include <random>

#include <bvh/binned_sah_builder.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/parallel_reinsertion_optimizer.hpp>
#include <bvh/node_layout_optimizer.hpp>

#include "bvh/bvh.hpp"
#include "bvh/single_ray_traverser.hpp"
#include "bvh/primitive_intersectors.hpp"
#include "bvh/triangle.hpp"
#include "bvh/vector.hpp"

#include "model_loaders/happly.hpp"
#include "model_loaders/tiny_obj_loader.hpp"

// CRT Imports:
#include "transform.hpp"

#include "lights/light.hpp"
#include "cameras/camera.hpp"
#include "lidars/lidar.hpp"

#include "materials/material.hpp"

#include "do_render.hpp"
#include "do_lidar.hpp"

#include "passes.hpp"

template <typename Scalar>
class BodyFixedGroup: public RigidBody<Scalar> {
    public:
        bvh::Bvh<Scalar> bvh_cache;
        std::vector<bvh::Triangle<Scalar>> triangles;

        Scalar scale;

        // Constructor:
        BodyFixedGroup(std::vector<Entity<Scalar>*> entities){
            // Store triangles locally:
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
            size_t reference_count = triangles.size();
            std::unique_ptr<bvh::Triangle<Scalar>[]> shuffled_triangles;

            std::cout << "\nBuilding Static BVH ( using SweepSahBuilder )... for " << triangles.size() << " triangles\n";
            using namespace std::chrono;
            auto start = high_resolution_clock::now();

            auto tri_data = triangles.data();
            auto bboxes_and_centers = bvh::compute_bounding_boxes_and_centers(tri_data, triangles.size());
            auto bboxes = bboxes_and_centers.first.get(); 
            auto centers = bboxes_and_centers.second.get(); 
            
            auto global_bbox = bvh::compute_bounding_boxes_union(bboxes, triangles.size());

            bvh::SweepSahBuilder<bvh::Bvh<Scalar>> builder(bvh_cache);
            builder.build(global_bbox, bboxes, centers, reference_count);

            bvh::ParallelReinsertionOptimizer<bvh::Bvh<Scalar>> pro_opt(bvh_cache);
            pro_opt.optimize();

            bvh::NodeLayoutOptimizer<bvh::Bvh<Scalar>> nlo_opt(bvh_cache);
            nlo_opt.optimize();

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(stop - start);
            std::cout << "    BVH of "
                << bvh_cache.node_count << " node(s) and "
                << reference_count << " reference(s)\n";
            std::cout << "    BVH built in " << duration.count()/1000000.0 << " seconds\n\n";

        }

        void set_scale(Scalar scale){
            this -> scale = scale;
        }

        std::vector<uint8_t> render(std::unique_ptr<Camera<Scalar>> &camera, std::vector<std::unique_ptr<Light<Scalar>>> &lights,
                                    int min_samples, int max_samples, Scalar noise_threshold, int num_bounces){
            auto image = do_render(camera, lights, bvh_cache, triangles, min_samples, max_samples, noise_threshold, num_bounces);
            return image;
        }

        Scalar simulate_lidar(std::unique_ptr<Lidar<Scalar>> &lidar, int num_rays){
            auto distance = do_lidar(lidar, this->bvh_cache, this->triangles, num_rays);
            return distance;
        }

        std::vector<Scalar> batch_simulate_lidar(std::unique_ptr<Lidar<Scalar>> &lidar, int num_rays){
            auto distances = do_batch_lidar(lidar, this->bvh_cache, this->triangles, num_rays);
            return distances;
        }

        std::vector<Scalar> intersection_pass(std::unique_ptr<Camera<Scalar>> &camera){
            auto intersections = get_inetersections<Scalar>(camera, this->bvh_cache, this->triangles);
            return intersections;
        }

        std::vector<uint32_t> instance_pass(std::unique_ptr<Camera<Scalar>> &camera){
            auto instances = get_instances<Scalar>(camera, this->bvh_cache, this->triangles);
            return instances;
        }

        std::vector<Scalar> normal_pass(std::unique_ptr<Camera<Scalar>> &camera){
            auto normals = get_normals<Scalar>(camera, this->bvh_cache, this->triangles);
            return normals;
        }
        
};

#endif