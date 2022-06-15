#ifndef __SCENE_H
#define __SCENE_H

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

#include "path_trace.hpp"
#include "passes.hpp"

#include "model_loaders/happly.hpp"
#include "model_loaders/tiny_obj_loader.hpp"

#include "transform.hpp"

#include "obj_temp/obj.hpp"
#include "materials/material.hpp"

#include "lights.hpp"
#include "cameras.hpp"

#include "materials/brdfs.hpp"

template <typename Scalar>
class StaticScene {
    public:
        bvh::Bvh<Scalar> bvh_cache;
        std::vector<bvh::Triangle<Scalar>> triangles;

        // Pose information
        bvh::Vector3<Scalar> position;
        Scalar rotation[3][3];

        // Constructor:
        StaticScene(std::vector<Entity<Scalar>*> entities){
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

            // Default values for all pose information:
            this -> position = bvh::Vector3<Scalar>(0,0,0);
            this -> rotation[0][0] = 1;
            this -> rotation[0][1] = 0;
            this -> rotation[0][2] = 0;
            this -> rotation[1][0] = 0;
            this -> rotation[1][1] = 1;
            this -> rotation[1][2] = 0;
            this -> rotation[2][0] = 0;
            this -> rotation[2][1] = 0;
            this -> rotation[2][2] = 1;
        }

        std::vector<uint8_t> render(std::unique_ptr<CameraModel<Scalar>> &camera, std::vector<std::unique_ptr<Light<Scalar>>> &lights,
                                    int min_samples, int max_samples, Scalar noise_threshold, int num_bounces){
            auto image = path_trace(camera, lights, bvh_cache, triangles, min_samples, max_samples, noise_threshold, num_bounces);
            return image;
        }

        std::vector<Scalar> intersection_pass(std::unique_ptr<CameraModel<Scalar>> &camera){
            auto intersections = get_inetersections<Scalar>(camera, bvh_cache, triangles);
            return intersections;
        }

        std::vector<uint32_t> instance_pass(std::unique_ptr<CameraModel<Scalar>> &camera){
            auto instances = get_instances<Scalar>(camera, bvh_cache, triangles);
            return instances;
        }

        std::vector<Scalar> normal_pass(std::unique_ptr<CameraModel<Scalar>> &camera){
            auto normals = get_normals<Scalar>(camera, bvh_cache, triangles);
            return normals;
        }
        
};

template <typename Scalar>
class StaticEntity {
    public:
        std::string geometry_path;
        std::string geometry_type;
        bool smooth_shading;
        Color color;

        bvh::Vector3<Scalar> position;
        Scalar rotation[3][3];
        Scalar scale;

        StaticEntity(std::string geometry_path, std::string geometry_type, bool smooth_shading, Color color){
            this->geometry_path = geometry_path;
            this->geometry_type = geometry_type;
            this->smooth_shading = smooth_shading;
            this->color = color;

            // Default values for all pose information:
            this -> scale = 1;
            this -> position = bvh::Vector3<Scalar>(0,0,0);
            this -> rotation[0][0] = 1;
            this -> rotation[0][1] = 0;
            this -> rotation[0][2] = 0;
            this -> rotation[1][0] = 0;
            this -> rotation[1][1] = 1;
            this -> rotation[1][2] = 0;
            this -> rotation[2][0] = 0;
            this -> rotation[2][1] = 0;
            this -> rotation[2][2] = 1;
        }

        // Pose setting methods:
        void set_scale(Scalar scale){
            this -> scale = scale;
        }
        void set_position(bvh::Vector3<Scalar> position) {
            this -> position = position;
        }
        void set_rotation(Scalar rotation[3][3]) {
            for (int i = 0; i < 3; i++){
                for (int j = 0; j <3; j++){
                    this -> rotation[i][j] = rotation[i][j];
                }
            }
        }
        void set_pose(bvh::Vector3<Scalar> position, Scalar rotation[3][3]){
            set_rotation(rotation);
            set_position(position);
        }
};

#endif