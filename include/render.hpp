#ifndef __RENDER_H
#define __RENDER_H

#include <cstdint>
#include <math.h>
#include <random>
#include <iomanip>

#include <bvh/bvh.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>
#include <bvh/triangle.hpp>

#include "lights.hpp"
#include "brdfs.hpp"
#include "cameras.hpp"

template <typename Scalar, typename Intersector>
Color illumination(bvh::SingleRayTraverser<bvh::Bvh<Scalar>> &traverser, Intersector &intersector, 
                                  float u, float v, const bvh::Ray<Scalar> &light_ray, 
                                  const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, Material<Scalar> *material) {
    Color intensity(0);
    auto hit = traverser.traverse(light_ray, intersector);
    if (!hit) {
        intensity = material->compute(light_ray, view_ray, normal, u, v);
    }
    return intensity;
}


template <typename Scalar>
void do_render(int max_samples, int min_samples, Scalar noise_threshold, int num_bounces, CameraModel<Scalar> &camera, 
            std::vector<PointLight<Scalar>> &point_lights, std::vector<SquareLight<Scalar>> &square_lights,
            const bvh::Bvh<Scalar>& bvh, const bvh::Triangle<Scalar>* triangles, float* pixels)
{
    bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> closest_intersector(bvh, triangles);
    bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> any_int(bvh, triangles);
    bvh::SingleRayTraverser<bvh::Bvh<Scalar>> traverser(bvh);

    // Initialize random number generator:
    std::random_device rd;
    std::minstd_rand eng(rd());
    std::uniform_real_distribution<Scalar> distr(-0.5, 0.5);
    std::uniform_real_distribution<Scalar> dist1(0.0, 1.0);

    size_t width  = (size_t) floor(camera.get_resolutionX());
    size_t height = (size_t) floor(camera.get_resolutionY());

    size_t done = 0;
    #pragma omp parallel for shared(done)
    for(size_t i = 0; i < width; ++i) {
        for(size_t j = 0; j < height; ++j) {
            size_t index = 3 * (width * j + i);
            // Loop through all samples for a given pixel:
            Color pixel_radiance(0);
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
                Color path_radiance(0);

                auto hit = traverser.traverse(ray, closest_intersector);

                // If no bouncesm, return just the vertex normal as the color:
                if (num_bounces == 0) {
                    if (hit) {
                        auto &tri = triangles[hit->primitive_index];
                        auto u = hit->intersection.u;
                        auto v = hit->intersection.v;
                        auto normal = tri.parent->interp_normals ? bvh::normalize(u*tri.vn1 + v*tri.vn2 + (Scalar(1.0)-u-v)*tri.vn0) : bvh::normalize(tri.n);
                        path_radiance[0] = std::abs(normal[0]);
                        path_radiance[1] = std::abs(normal[1]);
                        path_radiance[2] = std::abs(normal[2]);
                    }
                }

                // Loop through all bounces
                auto weight = Color(2*M_PI);
                for (int bounce = 0; bounce < num_bounces; ++bounce){
                    if (!hit) {
                        break;
                    }
                    auto &tri = triangles[hit->primitive_index];
                    auto u = hit->intersection.u;
                    auto v = hit->intersection.v;

                    auto normal = bvh::normalize(tri.n);
                    bvh::Vector3<Scalar> interp_normal;
                    if (tri.parent->interp_normals){
                        interp_normal = bvh::normalize(u*tri.vn1 + v*tri.vn2 + (Scalar(1.0)-u-v)*tri.vn0);
                    }
                    else {
                        interp_normal = normal;
                    }
                    bvh::Vector<float, 2> interp_uv = (float)u*tri.uv[1] + (float)v*tri.uv[2] + (float)(Scalar(1.0)-u-v)*tri.uv[0];
                    auto material = tri.parent->get_material(interp_uv[0], interp_uv[1]);

                    //TODO: Figure out how to deal with the self-intersection stuff in a more proper way...
                    bvh::Vector3<Scalar> intersect_point = (u*tri.p1() + v*tri.p2() + (1-u-v)*tri.p0);
                    Scalar scale = 0.0001;
                    intersect_point = intersect_point - scale*normal;

                    // Calculate the direct illumination:
                    Color light_radiance(0);
                    // Loop through all provided lights:
                    for (PointLight<Scalar> &light : point_lights){
                        bvh::Ray<Scalar> light_ray = light.sample_ray(intersect_point);
                        Color light_color = illumination(traverser, any_int, interp_uv[0], interp_uv[1], light_ray, ray, interp_normal, material);
                        light_radiance += light_color * (float) light.get_intensity(intersect_point);
                    };
                    for (SquareLight<Scalar> &light : square_lights){
                        bvh::Ray<Scalar> light_ray = light.sample_ray(intersect_point);
                        Color light_color = illumination(traverser, any_int, interp_uv[0], interp_uv[1], light_ray, ray, interp_normal, material);
                        light_radiance += light_color * (float) light.get_intensity(intersect_point);
                    };

                    if (bounce >= 1) {
                        for (int idx = 0; idx < 3; ++idx){
                            light_radiance[idx] = std::clamp(light_radiance[idx], float(0), float(1));
                        }
                    }

                    // Update the path radiance with the newly calculated radiance:
                    path_radiance += light_radiance*weight;

                    // Exit or cast next ray:
                    if (bounce == num_bounces-1) {
                        break;
                    }

                    auto [new_direction, bounce_color] = material->sample(ray, interp_normal, interp_uv[0], interp_uv[1]);

                    ray = bvh::Ray<Scalar>(intersect_point, new_direction);
                    hit = traverser.traverse(ray, closest_intersector);
                    weight *= bounce_color;
                }

                // Run adaptive sampling:
                auto rad_contrib = (path_radiance - pixel_radiance)*(1.0f/sample);
                pixel_radiance += rad_contrib;
                if (sample >= min_samples) {
                    Scalar noise = bvh::length(rad_contrib);
                    if (noise < noise_threshold) {
                        break;
                    }
                }
            }
            // Store the pixel intensity:
            pixels[index    ] = pixel_radiance[0];
            pixels[index + 1] = pixel_radiance[1];
            pixels[index + 2] = pixel_radiance[2];
        }

        #pragma omp critical
        std::cout << "Rendering " << std::setprecision(1) << std::fixed  << (100. * ++done) / width << "%..." << std::endl;
    }
}

#endif