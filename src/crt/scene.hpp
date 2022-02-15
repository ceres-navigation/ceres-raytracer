#ifndef __SCENE_H
#define __SCENE_H

#include "entity.hpp"
#include "render.hpp"

#include <bvh/binned_sah_builder.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/parallel_reinsertion_optimizer.hpp>
#include <bvh/node_layout_optimizer.hpp>

template <typename Scalar>
class Scene {
    public:       
        std::vector<Entity<Scalar>> entities;
        std::vector<bvh::Triangle<Scalar>> triangles;
        std::unique_ptr<CameraModel<Scalar>> camera;
        std::vector<std::unique_ptr<Light<Scalar>>> lights;

        int max_samples;
        int min_samples;
        Scalar noise_threshold;
        int num_bounces;

        Scene() 
        : max_samples(25), min_samples(3), noise_threshold(0.00001), num_bounces(2) {

        }

        void add_camera(std::unique_ptr<CameraModel<Scalar>> &camera){
            // this -> camera = std::make_unique<CameraModel<Scalar>>(camera);
            this->camera = std::move(camera);
        }

        void add_light(std::unique_ptr<Light<Scalar>> &light){
            this->lights.push_back(std::move(light));
        }

        void add_entity(Entity<Scalar> &entity){
            this->triangles.insert(this->triangles.end(), entity.triangles.begin(), entity.triangles.end());
            this->entities.push_back(entity);
        }

        void render(std::string out_file) {
            size_t width  = (size_t) floor((*camera).get_resolutionX());
            size_t height = (size_t) floor((*camera).get_resolutionY());

            bvh::Bvh<Scalar> bvh;

            size_t reference_count = triangles.size();
            std::unique_ptr<bvh::Triangle<Scalar>[]> shuffled_triangles;

            // Build an acceleration data structure for this object set
            std::cout << "\nBuilding BVH ( using SweepSahBuilder )..." << std::endl;
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

            auto pixels = std::make_unique<float[]>(3 * width * height);
            
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
            do_render(max_samples, min_samples, noise_threshold, num_bounces, *camera, lights, bvh, triangles.data(), pixels.get());
            stop = high_resolution_clock::now();
            duration = duration_cast<microseconds>(stop - start);
            std::cout << "    Tracing completed in " << duration.count()/1000000.0 << " seconds\n\n";

            Magick::Image image(Magick::Geometry(width,height), "green");
            image.modifyImage();
            Magick::Pixels view(image);
            Magick::Quantum *img_pix = view.set(0,0,width,height);

            for (size_t j = 0; j < 3 * width*height; j++) {
                *img_pix++ = std::clamp(pixels[j], 0.f, 1.f) * 65535;
            }

            view.sync();
            image.write(out_file);
        }   

};

#endif