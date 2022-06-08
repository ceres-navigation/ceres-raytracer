#ifndef __CERESRT_H
#define __CERESRT_H

#include "entity.hpp"
#include "render.hpp"

#include <bvh/binned_sah_builder.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/parallel_reinsertion_optimizer.hpp>
#include <bvh/node_layout_optimizer.hpp>

template <typename Scalar>
std::vector<uint8_t> render(std::unique_ptr<CameraModel<Scalar>> &camera_in, std::unique_ptr<Light<Scalar>> &lights_in, Entity<Scalar>*entity_in,
                            int min_samples, int max_samples, Scalar noise_threshold, int num_bounces) {
    min_samples = min_samples;
    max_samples = max_samples;
    noise_threshold = noise_threshold;
    num_bounces = num_bounces;
    std::vector<Entity<Scalar>*> entities;
    std::vector<bvh::Triangle<Scalar>> triangles;
    std::vector<std::unique_ptr<Light<Scalar>>> lights;

    // Ingest the input rendering objects:
    lights.push_back(std::move(lights_in));
    triangles.insert(triangles.end(), entity_in->triangles.begin(), entity_in->triangles.end());
    entities.push_back(entity_in);

    size_t width  = (size_t) floor(camera_in->get_resolutionX());
    size_t height = (size_t) floor(camera_in->get_resolutionY());

    bvh::Bvh<Scalar> bvh;

    size_t reference_count = triangles.size();
    std::unique_ptr<bvh::Triangle<Scalar>[]> shuffled_triangles;

    // Build an acceleration data structure for this object set
    std::cout << "\nBuilding BVH ( using SweepSahBuilder )... for " << triangles.size() << " triangles\n";
    using namespace std::chrono;
    auto start = high_resolution_clock::now();

    auto bboxes_and_centers = bvh::compute_bounding_boxes_and_centers(triangles.data(), triangles.size());
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
    
#ifdef _OPENMP
    #pragma omp parallel
    {
        #pragma omp single
        std::cout << "Rendering image on " << omp_get_num_threads() << " threads..." << std::endl;
    }
#else
    std::cout << "Rendering image on single thread..." << std::endl;
#endif

    start = high_resolution_clock::now();
    do_render(max_samples, min_samples, noise_threshold, num_bounces, *camera_in, lights, bvh, triangles.data(), pixels.get());
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    std::cout << "    Tracing completed in " << duration.count()/1000000.0 << " seconds\n\n";

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

#endif