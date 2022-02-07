#ifndef __SCENE_H
#define __SCENE_H

#include "entity.hpp"
#include "render.hpp"

template <typename Scalar>
class Scene {
    using Bvh = bvh::Bvh<Scalar>;
    using Triangle = bvh::Triangle<Scalar>;
    using Vector3 = bvh::Vector3<Scalar>;

    private:        

        std::vector<std::unique_ptr<Entity<Scalar>>> entities;
        std::vector<Triangle> triangles;
        std::vector<PointLight<Scalar>> point_lights;
        std::vector<SquareLight<Scalar>> square_lights;

    public:

        int max_samples;
        int min_samples;
        Scalar noise_threshold;
        int num_bounces;

        Scene() 
        : max_samples(25), min_samples(3), noise_threshold(0.00001), num_bounces(2) {

        }

        void add_triangles(std::vector<Triangle> &tri) {
            triangles.insert(triangles.end(), tri.begin(), tri.end());
        }

        void add_entity(Entity<Scalar> ent, Scalar rotation[3][3], bvh::Vector3<Scalar> position, Scalar scale) {
            entities.emplace_back(new Entity(ent));
            auto new_tri = entities.rbegin()->get()->get_triangles();
            triangles.reserve(triangles.size() + new_tri.size());
            
            for (auto &tri : new_tri) {
                // Transform each of the vertices:
                auto p0 = transform(tri.p0, rotation, position, scale);
                auto p1 = transform(tri.p1(), rotation, position, scale);
                auto p2 = transform(tri.p2(), rotation, position, scale);

                // Transform each of the vertex normals:
                auto vn0 = rotate(tri.vn0, rotation);
                auto vn1 = rotate(tri.vn1, rotation);
                auto vn2 = rotate(tri.vn2, rotation);

                // Ok, somebody needs to fix the copy constructor...
                auto vc0 = tri.vc[0];
                auto vc1 = tri.vc[1];
                auto vc2 = tri.vc[2];

                auto uv0 = tri.uv[0];
                auto uv1 = tri.uv[1];
                auto uv2 = tri.uv[2];

                triangles.emplace_back(p0, p1, p2);
                triangles.rbegin()->add_vetex_normals(vn0, vn1, vn2);
                triangles.rbegin()->add_vertex_colors(vc0, vc1, vc2);
                triangles.rbegin()->add_vertex_uv(uv0, uv1, uv2);
                triangles.rbegin()->set_parent(entities.rbegin()->get());
            }
        }

        void add_point_light(PointLight<Scalar> &light) {
            point_lights.push_back(light);
        }

        void add_square_light(SquareLight<Scalar> &light) {
            square_lights.push_back(light);
        }

        void render(CameraModel<Scalar> &camera, std::string out_file) {
            size_t width  = (size_t) floor(camera.get_resolutionX());
            size_t height = (size_t) floor(camera.get_resolutionY());

            Bvh bvh;

            size_t reference_count = triangles.size();
            std::unique_ptr<Triangle[]> shuffled_triangles;

            // Build an acceleration data structure for this object set
            std::cout << "\nBuilding BVH ( using BinnedSahBuilder )..." << std::endl;
            using namespace std::chrono;
            auto start = high_resolution_clock::now();

            auto bboxes_and_centers = bvh::compute_bounding_boxes_and_centers(triangles.data(), triangles.size());
            auto bboxes = bboxes_and_centers.first.get(); 
            auto centers = bboxes_and_centers.second.get(); 
            
            auto global_bbox = bvh::compute_bounding_boxes_union(bboxes, triangles.size());

            bvh::BinnedSahBuilder<Bvh, 16> builder(bvh);
            builder.build(global_bbox, bboxes, centers, reference_count);

            auto stop = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(stop - start);
            std::cout << "    BVH of "
                << bvh.node_count << " node(s) and "
                << reference_count << " reference(s)\n";
            std::cout << "    BVH built in " << duration.count()/1000000.0 << " seconds\n\n";

            auto pixels = std::make_unique<Scalar[]>(3 * width * height);
            
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
            do_render(max_samples, min_samples, noise_threshold, num_bounces, camera, point_lights, square_lights, bvh, triangles.data(), pixels.get());
            stop = high_resolution_clock::now();
            duration = duration_cast<microseconds>(stop - start);
            std::cout << "    Tracing completed in " << duration.count()/1000000.0 << " seconds\n\n";

            Magick::Image image(Magick::Geometry(width,height), "green");
            image.modifyImage();
            Magick::Pixels view(image);
            Magick::Quantum *img_pix = view.set(0,0,width,height);

            for (size_t j = 0; j < 3 * width*height; j++) {
                *img_pix++ = pixels[j] * 65535;
            }

            view.sync();
            image.write(out_file);
        }   

};

#endif