#ifndef __RENDER_H
#define __RENDER_H

#include <cstdint>
#include <math.h>
#include <random>

#include <bvh/bvh.hpp>
#include <bvh/binned_sah_builder.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>
#include <bvh/triangle.hpp>

#include "lights.hpp"
#include "brdfs.hpp"
#include "cameras.hpp"

template <typename Scalar>
bvh::Vector3<Scalar> illumination(bvh::SingleRayTraverser<bvh::Bvh<Scalar>> &traverser, 
                                 bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> &intersector, 
                                 Scalar &u, Scalar &v, bvh::Triangle<Scalar> &tri, bvh::Ray<Scalar> light_ray) {
    bvh::Vector3<Scalar> intensity;
    // Loop through all provided lights:
    auto hit = traverser.traverse(light_ray, intersector);
    if (!hit) {
        auto normal = tri.parent->interp_normals ? u*tri.vn1 + v*tri.vn2 + (1-u-v)*tri.vn0 : tri.n;
        bvh::Vector<float, 2> interp_uv = (float)u*tri.uv[1] + (float)v*tri.uv[2] + (float)(1-u-v)*tri.uv[0];
        auto c = tri.parent->get_material(interp_uv[0], interp_uv[1])->get(light_ray, normal, interp_uv[0], interp_uv[1]);
        intensity[0] = c[0];
        intensity[1] = c[1];
        intensity[2] = c[2];
    } else{
        intensity = bvh::Vector3<Scalar>(0.0,0.0,0.0);
    }
    return intensity;
}


template <typename Scalar>
void do_render(int max_samples, int min_samples, Scalar noise_threshold, int num_bounces, CameraModel<Scalar> &camera, 
            std::vector<PointLight<Scalar>> &point_lights, std::vector<SquareLight<Scalar>> &square_lights,
            const bvh::Bvh<Scalar>& bvh, const bvh::Triangle<Scalar>* triangles, Scalar* pixels)
{
    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> intersector(bvh, triangles);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh);

    // Initialize random number generator:
    std::random_device rd;
    std::minstd_rand eng(rd());
    std::uniform_real_distribution<Scalar> distr(-0.5, 0.5);
    std::uniform_real_distribution<Scalar> dist1(0.0, 1.0);

    size_t width  = (size_t) floor(camera.get_resolutionX());
    size_t height = (size_t) floor(camera.get_resolutionY());

    #pragma omp parallel for
    for(size_t i = 0; i < width; ++i) {
        for(size_t j = 0; j < height; ++j) {
            size_t index = 3 * (width * j + i);
            // Loop through all samples for a given pixel:
            bvh::Vector3<Scalar> pixel_radiance(0.0,0.0,0.0);
            for (int sample = 1; sample < max_samples+1; ++sample) {
                // TODO: Make a better random sampling algorithm:
                bvh::Ray<Scalar> ray;
                auto i_rand = distr(eng);
                auto j_rand = distr(eng);
                if (max_samples == 1) {
                    ray = camera.pixel_to_ray(i, j);
                }
                else {
                    ray = camera.pixel_to_ray(i + i_rand, j + j_rand);
                }
                bvh::Vector3<Scalar> path_radiance(0.0,0.0,0.0);

                auto hit = traverser.traverse(ray, intersector);

                // If no bouncesm, return just the vertex normal as the color:
                if (num_bounces == 0) {
                    if (hit) {
                        auto tri = triangles[hit->primitive_index];
                        auto normal = bvh::normalize(tri.n);
                        path_radiance[0] = normal[0];
                        path_radiance[1] = normal[1];
                        path_radiance[2] = normal[2];
                    }
                }

                // Loop through all bounces
                Scalar weight = 2*M_PI;
                for (int bounce = 0; bounce < num_bounces; ++bounce){
                    if (!hit) {
                        break;
                    }
                    auto tri = triangles[hit->primitive_index];
                    auto normal = bvh::normalize(tri.n);

                    //TODO: Figure out how to deal with the self-intersection stuff in a more proper way...
                    auto u = hit->intersection.u;
                    auto v = hit->intersection.v;
                    bvh::Vector3<Scalar> intersect_point = (u*tri.p1() + v*tri.p2() + (1-u-v)*tri.p0);
                    Scalar scale = -0.0001;
                    intersect_point = intersect_point + scale*normal;

                    // Calculate the direct illumination:
                    bvh::Vector3<Scalar> light_radiance(0.0,0.0,0.0);
                    for (PointLight<Scalar> &light : point_lights){
                        bvh::Ray<Scalar> light_ray = light.sample_ray(intersect_point);
                        bvh::Vector3<Scalar> light_radiance_new = illumination<Scalar>(traverser, intersector, u, v, tri, light_ray);
                        light_radiance += light_radiance_new * light.get_intensity(intersect_point);
                    };
                    for (SquareLight<Scalar> &light : square_lights){
                        bvh::Ray<Scalar> light_ray = light.sample_ray(intersect_point);
                        bvh::Vector3<Scalar> light_radiance_new = illumination<Scalar>(traverser, intersector, u, v, tri, light_ray);
                        light_radiance += light_radiance_new * light.get_intensity(intersect_point);
                    };

                    // Update the path radiance with the newly calculated radiance:
                    path_radiance += light_radiance*weight;

                    // Exit or cast next ray:
                    if (bounce == num_bounces-1) {
                        break;
                    }

                    //TODO: Move this into a class contained by a parent object to a triangle:
                    Scalar r1 = dist1(eng);
                    Scalar r2 = dist1(eng);
                    bvh::Ray<Scalar> ray = cosine_importance(intersect_point, -normal, r1, r2);
                    hit = traverser.traverse(ray, intersector);
                    weight *= r1; // cos(theta)
                }

                // Run adaptive sampling:
                auto rad_contrib = (path_radiance - pixel_radiance)*Scalar(1.0/sample);
                pixel_radiance += rad_contrib;
                if (sample >= min_samples) {
                    Scalar noise = bvh::length(rad_contrib);
                    if (noise < noise_threshold) {
                        break;
                    }
                }
            }
            // Store the pixel intensity:
            pixels[index    ] = std::fabs(pixel_radiance[0]);
            pixels[index + 1] = std::fabs(pixel_radiance[1]);
            pixels[index + 2] = std::fabs(pixel_radiance[2]);
        }
    }
}

#endif