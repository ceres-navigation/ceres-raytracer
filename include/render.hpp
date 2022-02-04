#ifndef __RENDER_H
#define __RENDER_H

#include <cstdint>

#include <bvh/bvh.hpp>
#include <bvh/binned_sah_builder.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>
#include <bvh/triangle.hpp>

#include "transform.hpp"
#include "brdfs.hpp"
#include "cameras.hpp"

// using Vector3 =  bvh::Vector3<Scalar>;
// using Bvh =  bvh::Bvh<Scalar>;
// using Triangle =  bvh::Triangle<Scalar>;

template <typename Scalar>
std::pair<int, int> render(const Camera<Scalar>& camera, const bvh::Vector3<Scalar>& sun_position, const bvh::Bvh<Scalar>& bvh,
            const bvh::Triangle<Scalar>* triangles, Scalar* pixels,
            size_t width, size_t height)
{
    auto dir = bvh::normalize(camera.dir);
    auto image_u = bvh::normalize(bvh::cross(dir, camera.up));
    auto image_v = bvh::normalize(bvh::cross(image_u, dir));
    auto image_w = std::tan(camera.fov * Scalar(3.14159265 * (1.0 / 180.0) * 0.5));
    auto ratio = Scalar(height) / Scalar(width);
    image_u = image_u * image_w;
    image_v = image_v * image_w * ratio;

    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> intersector(bvh, triangles);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh);

    size_t traversal_steps = 0, intersections = 0;

    #pragma omp parallel for collapse(2) reduction(+: traversal_steps, intersections)
    for(size_t i = 0; i < width; ++i) {
        for(size_t j = 0; j < height; ++j) {
            size_t index = 3 * (width * j + i);

            auto u = 2 * (i + Scalar(0.5)) / Scalar(width)  - Scalar(1);
            auto v = 2 * (j + Scalar(0.5)) / Scalar(height) - Scalar(1);

            bvh::Ray<Scalar> ray(camera.eye, bvh::normalize(image_u * u + image_v * v + dir));
            auto hit = traverser.traverse(ray, intersector);
            traversal_steps++;
            if (!hit) {
                pixels[index] = pixels[index + 1] = pixels[index + 2] = 0;
            } else {
                intersections++;
                auto tri = triangles[hit->primitive_index];
                auto normal = bvh::normalize(tri.n);
                // pixels[index    ] = std::fabs(normal[0]);
                // pixels[index + 1] = std::fabs(normal[1]);
                // pixels[index + 2] = std::fabs(normal[2]);

                auto u = hit->intersection.u;
                auto v = hit->intersection.v;
                bvh::Vector3<Scalar> intersect_point = (u*tri.p0 + v*tri.p1() + (1-u-v)*tri.p2());

                //TODO: Figure out how to deal with the self-intersection stuff in a more proper way...
                Scalar scale = -0.00001;
                intersect_point = intersect_point + scale*normal;

                bvh::Vector3<Scalar> sun_line = bvh::normalize(sun_position - intersect_point);
                bvh::Ray<Scalar> ray(intersect_point, sun_line);
                auto hit = traverser.traverse(ray, intersector);
                traversal_steps++;
                if (!hit) {
                    // Calculate the shading:
                    float reflected_intensity;
                    lambertian(sun_line, normal, reflected_intensity);
                    pixels[index    ] = std::fabs(reflected_intensity) * 1.00;
                    pixels[index + 1] = std::fabs(reflected_intensity) * 1.00;
                    pixels[index + 2] = std::fabs(reflected_intensity) * 1.00;
                } else{
                    intersections++;
                    pixels[index] = pixels[index + 1] = pixels[index + 2] = 0;
                }
            }
        }
    }

    return std::pair(traversal_steps, intersections);
}

#endif