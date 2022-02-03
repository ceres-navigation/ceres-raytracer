#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <chrono>

#include <bvh/bvh.hpp>
#include <bvh/binned_sah_builder.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>
#include <bvh/triangle.hpp>

using Scalar      = float;
using Vector3     = bvh::Vector3<Scalar>;
using Triangle    = bvh::Triangle<Scalar>;
using BoundingBox = bvh::BoundingBox<Scalar>;
using Ray         = bvh::Ray<Scalar>;
using Bvh         = bvh::Bvh<Scalar>;

#include <obj.hpp>

struct Camera {
    Vector3 eye;
    Vector3 dir;
    Vector3 up;
    Scalar  fov;
};

void lambertian(Vector3 sun_line, Vector3 normal, float &reflected_intensity){
    reflected_intensity = sun_line[0]*normal[0] + sun_line[1]*normal[1] + sun_line[2]*normal[2];
    return;
};

void render(const Camera& camera, const Vector3& sun_position, const Bvh& bvh,
            const Triangle* triangles, Scalar* pixels,
            size_t width, size_t height)
{
    auto dir = bvh::normalize(camera.dir);
    auto image_u = bvh::normalize(bvh::cross(dir, camera.up));
    auto image_v = bvh::normalize(bvh::cross(image_u, dir));
    auto image_w = std::tan(camera.fov * Scalar(3.14159265 * (1.0 / 180.0) * 0.5));
    auto ratio = Scalar(height) / Scalar(width);
    image_u = image_u * image_w;
    image_v = image_v * image_w * ratio;

    bvh::ClosestPrimitiveIntersector<Bvh, Triangle, false> intersector(bvh, triangles);
    bvh::SingleRayTraverser<Bvh> traverser(bvh);

    size_t traversal_steps = 0, intersections = 0;

    #pragma omp parallel for collapse(2) reduction(+: traversal_steps, intersections)
    for(size_t i = 0; i < width; ++i) {
        for(size_t j = 0; j < height; ++j) {
            size_t index = 3 * (width * j + i);

            auto u = 2 * (i + Scalar(0.5)) / Scalar(width)  - Scalar(1);
            auto v = 2 * (j + Scalar(0.5)) / Scalar(height) - Scalar(1);

            Ray ray(camera.eye, bvh::normalize(image_u * u + image_v * v + dir));
            auto hit = traverser.traverse(ray, intersector);
            if (!hit) {
                pixels[index] = pixels[index + 1] = pixels[index + 2] = 0;
            } else {
                auto tri = triangles[hit->primitive_index];
                auto normal = bvh::normalize(tri.n);
                // pixels[index    ] = std::fabs(normal[0]);
                // pixels[index + 1] = std::fabs(normal[1]);
                // pixels[index + 2] = std::fabs(normal[2]);

                float u = hit->intersection.u;
                float v = hit->intersection.v;
                Vector3 intersect_point = (u*tri.p0 + v*tri.p1() + (1-u-v)*tri.p2());

                //TODO: Figure out how to deal with the self-intersection stuff in a more proper way...
                float scale = 1.01;
                intersect_point[0] = intersect_point[0]*scale;
                intersect_point[1] = intersect_point[1]*scale;
                intersect_point[2] = intersect_point[2]*scale;

                Vector3 sun_line = bvh::normalize(sun_position - intersect_point);
                Ray ray(intersect_point, sun_line);
                auto hit = traverser.traverse(ray, intersector);
                if (!hit) {
                    // Calculate the shading:
                    float reflected_intensity;
                    lambertian(sun_line, normal, reflected_intensity);
                    pixels[index    ] = std::fabs(reflected_intensity);
                    pixels[index + 1] = std::fabs(reflected_intensity);
                    pixels[index + 2] = std::fabs(reflected_intensity);
                } else{
                    pixels[index] = pixels[index + 1] = pixels[index + 2] = 0;
                }
            }
        }
    }
}

template <size_t Axis>
static void rotate_triangles(Scalar degrees, Triangle* triangles, size_t triangle_count) {
    static constexpr Scalar pi = Scalar(3.14159265359);
    auto cos = std::cos(degrees * pi / Scalar(180));
    auto sin = std::sin(degrees * pi / Scalar(180));
    auto rotate = [&] (const Vector3& p) {
        if (Axis == 0)
            return Vector3(p[0], p[1] * cos - p[2] * sin, p[1] * sin + p[2] * cos);
        else if (Axis == 1)
            return Vector3(p[0] * cos + p[2] * sin, p[1], -p[0] * sin + p[2] * cos);
        else
            return Vector3(p[0] * cos - p[1] * sin, p[0] * sin + p[1] * cos, p[2]);
    };
    #pragma omp parallel for
    for (size_t i = 0; i < triangle_count; ++i) {
        auto p0 = rotate(triangles[i].p0);
        auto p1 = rotate(triangles[i].p1());
        auto p2 = rotate(triangles[i].p2());
        triangles[i] = Triangle(p0, p1, p2);
    }
}


int main(int argc, char** argv) {

    const char* output_file  = "render.ppm";
    // const char* input_file   = "Bennu_v20_200k.obj";
    // Camera camera = {
    //     Vector3(0.0,  -1.0, 0.0),
    //     Vector3(0, 1, 0),
    //     Vector3(0, 0, 1),
    //     60
    // };
    // Vector3 sun_position = Vector3(100.0, 0, 0.0);
    // size_t rotation_axis = 0;
    // Scalar rotation_degrees = 0;

    const char* input_file = "../../data/dragon.obj";
    Camera camera = {
        Vector3(0.0, -15.0, 2.0),
        Vector3(0, 1, 0),
        Vector3(0, 0, 1),
        60
    };
    Vector3 sun_position = Vector3(-50.0, -20.0, 0.0);
    size_t rotation_axis = 0;
    Scalar rotation_degrees = 90;

    // const char* input_file = "dragon.obj";
    // Camera camera = {
    //     Vector3(-2.0, 0.0, 0.0),
    //     Vector3(1, 0, 0),
    //     Vector3(0, 0, 1),
    //     60
    // };
    // Vector3 sun_position = Vector3(-50.0, 100.0, 0.0);
    // size_t rotation_axis = 0;
    // Scalar rotation_degrees = 90;

    // const char* input_file = "bike.obj";
    // Camera camera = {
    //     Vector3(-2.0, 0.0, 0.5),
    //     Vector3(1, 0, 0),
    //     Vector3(0, 0, 1),
    //     60
    // };
    // Vector3 sun_position = Vector3(-50.0, 100.0, 0.0);
    // size_t rotation_axis = 0;
    // Scalar rotation_degrees = 90;
    

    size_t width  = 1920;
    size_t height = 1080;

    // Load mesh from file
    auto triangles = obj::load_from_file(input_file);
    if (triangles.size() == 0) {
        std::cerr << "The given scene is empty or cannot be loaded" << std::endl;
        return 1;
    }

    // Rotate triangles if requested
    if (rotation_axis == 0)
        rotate_triangles<0>(rotation_degrees, triangles.data(), triangles.size());
    else if (rotation_axis == 1)
        rotate_triangles<1>(rotation_degrees, triangles.data(), triangles.size());
    else if (rotation_axis == 2)
        rotate_triangles<2>(rotation_degrees, triangles.data(), triangles.size());

    Bvh bvh;

    size_t reference_count = triangles.size();
    std::unique_ptr<Triangle[]> shuffled_triangles;

    // Build an acceleration data structure for this object set
    std::cout << "Building BVH ( using BinnedSahBuilder )..." << std::endl;
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
    std::cout << duration.count()/1000000.0 << std::endl;

    std::cout << "BVH of "
        << bvh.node_count << " node(s) and "
        << reference_count << " reference(s)" << std::endl;

    auto pixels = std::make_unique<Scalar[]>(3 * width * height);

    #pragma omp parallel
    {
        #pragma omp single
        std::cout << "Rendering image (" << width << "x" << height << ") on " << omp_get_num_threads() << " threads..." << std::endl;
    }
    // std::cout << "Rendering image (" << width << "x" << height << ")..." << std::endl;
    auto start2 = high_resolution_clock::now();
    render(camera, sun_position, bvh, triangles.data(), pixels.get(), width, height);
    auto stop2 = high_resolution_clock::now();
    auto duration2 = duration_cast<microseconds>(stop2 - start2);
    std::cout << duration2.count()/1000000.0 << std::endl;

    std::ofstream out(output_file, std::ofstream::binary);
    out << "P6 " << width << " " << height << " " << 255 << "\n";
    for(size_t j = height; j > 0; --j) {
        for(size_t i = 0; i < width; ++i) {
            size_t index = 3* (width * (j - 1) + i);
            uint8_t pixel[3] = {
                static_cast<uint8_t>(std::max(std::min(pixels[index    ] * 255, Scalar(255)), Scalar(0))),
                static_cast<uint8_t>(std::max(std::min(pixels[index + 1] * 255, Scalar(255)), Scalar(0))),
                static_cast<uint8_t>(std::max(std::min(pixels[index + 2] * 255, Scalar(255)), Scalar(0)))
            };
            out.write(reinterpret_cast<char*>(pixel), sizeof(uint8_t) * 3);
        }
    }
}