#ifndef __UVMAPS_H
#define __UVMAPS_H

using Color = bvh::Vector3<float>;

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


#endif