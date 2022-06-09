#ifndef __MATERIAL_H
#define __MATERIAL_H

#include <memory>
#include <vector>
#include <random>

#include "lodepng/lodepng.h"

#include "bvh/triangle.hpp"
#include "bvh/vector.hpp"

#include "brdfs.hpp"

using Color = bvh::Vector3<float>;

static Color inline clamp_color(Color c) {
    return(Color(std::clamp(c[0], 0.f, 1.f), std::clamp(c[1], 0.f, 1.f), std::clamp(c[2], 0.f, 1.f)));
}

template <typename Value>
class UVMap {
    public:
    virtual Value operator()(float u, float v) = 0;
};

template <typename Value>
class ConstantUVMap : public  UVMap<Value> {
    private:
    Value val;
   
    public:
    ConstantUVMap(Value val) : val(val) { }
    Value operator()(float u, float v) { return val; };
};

class ImageUVMap : public UVMap<Color> {
    private:

    unsigned int width;
    unsigned int height;
    std::vector<uint8_t> pixels;
 
    public:
    ImageUVMap(std::string path) { 
        unsigned error = lodepng::decode(pixels, width, height, path);
        if(error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
     }
 
    Color operator()(float u, float v) {
        size_t x = (size_t)(u * width + 0.5);
        size_t y = (size_t)(v * height + 0.5);

        size_t idx = 4 * width * y + 4 * x;

        return Color(
            pixels[idx+0] / 255.f,
            pixels[idx+1] / 255.f,
            pixels[idx+2] / 255.f
        );
    };
};

class ImageIndexUVMap : public UVMap<size_t> {
    private:
    size_t range;
    ImageUVMap internal;
    
    public:
    ImageIndexUVMap(std::string path, size_t range = 1) : range(range), internal(path) { }
    
    size_t operator()(float u, float v) {
        return (size_t)(internal(u, v)[0] * range + 0.5);
    };
};

template <typename Scalar>
class Material {
    public:
    virtual Color compute(const bvh::Ray<Scalar> &light_ray, const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, float u, float v) = 0;
    virtual std::pair<bvh::Vector3<Scalar>, Color> sample(const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, float u, float v) = 0;
    
    Color get(bvh::Ray<Scalar> light_ray, bvh::Vector3<Scalar> normal, float u, float v) {
        std::cout << "hello from inside materials\n";
        return this->compute(light_ray, light_ray, normal, u, v);
    };

};

template <typename Scalar>
class ColoredLambertianMaterial : public Material<Scalar> {
    private:
    Color c;
    std::minstd_rand eng;
    std::uniform_real_distribution<Scalar> d;
    
    public:
    ColoredLambertianMaterial(Color color) : c(color), eng(std::random_device()()), d(0.0, 1.0) { }

    Color compute(const bvh::Ray<Scalar> &light_ray, const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, float u, float v) {
        auto L_dot_N = -bvh::dot(light_ray.direction, normal);
        return c * (float)(L_dot_N);
    }

    std::pair<bvh::Vector3<Scalar>, Color> sample(const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, float u, float v) {
        auto r1 = d(eng);
        auto r2 = d(eng);
        auto dir = cosine_importance(normal, r1, r2);
        return std::make_pair(dir, (float)(1-r1)*Color(1));
    }
};

template <typename Scalar>
class TexturedLambertianMaterial : public Material<Scalar> {
    private:
    std::shared_ptr<UVMap<Color>> tex_map;
    std::minstd_rand eng;
    std::uniform_real_distribution<Scalar> d;
   
    public:
    TexturedLambertianMaterial(std::shared_ptr<UVMap<Color>> texture) 
    : tex_map(texture), eng(std::random_device()()), d(0.0, 1.0) { }

    Color compute(const bvh::Ray<Scalar> &light_ray, const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, float u, float v) {
        auto L_dot_N = -bvh::dot(light_ray.direction, normal);
        return (*tex_map)(u, v) * (float)(L_dot_N);
    }

    std::pair<bvh::Vector3<Scalar>, Color> sample(const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, float u, float v) {
        auto r1 = d(eng);
        auto r2 = d(eng);
        auto dir = cosine_importance(normal, r1, r2);
        return std::make_pair(dir, (float)(1-r1)*Color(1));
    }
};

template <typename Scalar>
class TexturedBlinnPhongMaterial : public Material<Scalar> {
    private:
    std::shared_ptr<UVMap<Color>> tex_map;
    std::shared_ptr<UVMap<Color>> spec_map;
    Scalar alpha;
    std::minstd_rand eng;
    std::uniform_real_distribution<Scalar> d;
   
    public:
    TexturedBlinnPhongMaterial(std::shared_ptr<UVMap<Color>> texture, std::shared_ptr<UVMap<Color>> specular, Scalar alpha = 24) 
    : tex_map(texture), spec_map(specular), alpha(alpha), eng(std::random_device()()), d(0.0, 1.0) { }

    Color compute(const bvh::Ray<Scalar> &light_ray, const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, float u, float v) {
        float diffuse = -static_cast<float>(bvh::dot(light_ray.direction, normal));
        auto color = (*tex_map)(u, v);
        auto coeffs = (*spec_map)(u, v);
        auto ka = coeffs[0];
        auto kd = coeffs[1];
        auto ks = coeffs[2];
        float spec = static_cast<float>(std::pow(bvh::dot(bvh::normalize(normal), bvh::normalize(-view_ray.direction + light_ray.direction)), alpha));
        return clamp_color(color*(ka + kd*diffuse) + ks*spec*Color(1));
    }

    std::pair<bvh::Vector3<Scalar>, Color> sample(const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, float u, float v) {
        auto r1 = d(eng);
        auto r2 = d(eng);
        auto dir = cosine_importance(normal, r1, r2);
        return std::make_pair(dir, (float)(1-r1)*Color(1));
    }
};

template <typename Scalar>
class MirrorMaterial : public Material<Scalar> {
    private:
    
    public:
    MirrorMaterial() { }

    Color compute(const bvh::Ray<Scalar> &light_ray, const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, float u, float v) {
        return Color(0);
    }

    std::pair<bvh::Vector3<Scalar>, Color> sample(const bvh::Ray<Scalar> &view_ray, const bvh::Vector3<Scalar> &normal, float u, float v) {
        // Is this right?
        return std::make_pair(view_ray.direction - normal * Scalar(2) * bvh::dot(view_ray.direction, normal), Color(1));
    }
};

#endif