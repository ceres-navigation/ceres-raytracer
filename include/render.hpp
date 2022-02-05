#ifndef __RENDER_H
#define __RENDER_H

#include <cstdint>
#include <math.h>

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
std::pair<int, int> render(CameraModel<Scalar> &camera, const bvh::Vector3<Scalar>& sun_position, const bvh::Bvh<Scalar>& bvh,
            const bvh::Triangle<Scalar>* triangles, Scalar* pixels)
{
    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> intersector(bvh, triangles);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh);

    size_t width  = (size_t) floor(camera.get_resolutionX());
    size_t height = (size_t) floor(camera.get_resolutionY());

    size_t traversal_steps = 0, intersections = 0;

    #pragma omp parallel for collapse(2) reduction(+: traversal_steps, intersections)
    for(size_t i = 0; i < width; ++i) {
        for(size_t j = 0; j < height; ++j) {
            size_t index = 3 * (width * j + i);

            bvh::Ray<Scalar> ray = camera.pixel_to_ray(i,j);
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
                    // // Calculate the shading:
                    float reflected_intensity;

                    //TODO: Add smooth shading as an option per triangle:
                    bvh::Vector3<Scalar> interpolated_normal = u*tri.vn1 + v*tri.vn2 + (1-u-v)*tri.vn0;
                    lambertian(sun_line, interpolated_normal, reflected_intensity);

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