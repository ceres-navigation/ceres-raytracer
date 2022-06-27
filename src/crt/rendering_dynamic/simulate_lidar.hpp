#ifndef __SIMULATE_LIDAR_H
#define __SIMULATE_LIDAR_H

#include <bvh/bvh.hpp>

template <typename Scalar> 
Scalar simulate_lidar(std::unique_ptr<Lidar<Scalar>> &lidar, 
                      std::vector<Entity<Scalar>*> entities,
                      int num_rays){

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

    auto distance = do_lidar<Scalar>(lidar, bvh, triangles, num_rays);

    return distance;
};

#endif