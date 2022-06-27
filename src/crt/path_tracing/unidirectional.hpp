#ifndef __UNIDIRECTIONAL_H
#define __UNIDIRECTIONAL_H

#include "bvh/bvh.hpp"
#include "bvh/single_ray_traverser.hpp"
#include "bvh/primitive_intersectors.hpp"
#include "bvh/triangle.hpp"

#include "lights/light.hpp"

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
Color unidirectional(std::vector<std::unique_ptr<Light<Scalar>>> &lights,
                     bvh::Triangle<Scalar>* tri_data,
                     bvh::ClosestPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> &closest_intersector,
                     bvh::AnyPrimitiveIntersector<bvh::Bvh<Scalar>, bvh::Triangle<Scalar>, false> &any_intersector,
                     bvh::SingleRayTraverser<bvh::Bvh<Scalar>> &traverser,
                     bvh::Ray<Scalar> ray, int num_bounces){
    
    // TODO: Make a better random sampling algorithm:
    auto hit = traverser.traverse(ray, closest_intersector);

    // Initialize:
    Color path_radiance(0);

    // Loop through all bounces
    auto weight = Color(2*M_PI);
    int bounce = 0;
    for (; bounce < num_bounces; ++bounce){
        if (!hit) {
            break;
        }
        auto &tri = tri_data[hit->primitive_index];
        auto u = hit->intersection.u;
        auto v = hit->intersection.v;

        auto normal = bvh::normalize(tri.n);
        bvh::Vector3<Scalar> interp_normal;
        if (tri.parent->smooth_shading){
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
        for (auto& light : lights){
            bvh::Ray<Scalar> light_ray = light->sample_ray(intersect_point);
            Color light_color = illumination(traverser, any_intersector, interp_uv[0], interp_uv[1], light_ray, ray, interp_normal, material);
            light_radiance += light_color * (float) light->get_intensity(intersect_point);
        };

        if (bounce >= 1) {
            for (int idx = 0; idx < 3; ++idx){
                light_radiance[idx] = std::clamp(light_radiance[idx], float(0), float(1));
            }
        }

        // Update the path radiance with the newly calculated radiance:
        path_radiance += light_radiance*weight;

        // Exit if max bounces is reached:
        if (bounce == num_bounces-1) {
            break;
        }

        // Cast next ray:
        auto [new_direction, bounce_color] = material->sample(ray, interp_normal, interp_uv[0], interp_uv[1]);
        ray = bvh::Ray<Scalar>(intersect_point, new_direction);
        hit = traverser.traverse(ray, closest_intersector);
        weight *= bounce_color;
    }

    return path_radiance;
}

#endif