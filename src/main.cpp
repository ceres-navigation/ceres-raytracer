#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <chrono>

#include <bvh/bvh.hpp>
#include <bvh/binned_sah_builder.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/primitive_intersectors.hpp>
#include <bvh/triangle.hpp>

#include <Magick++.h> 
// #include <toml.hpp>
// #include <cpptoml.h>
#include <INIReader.h>

#include "rotations.hpp"
#include "render.hpp"
#include "obj.hpp"

template <typename Scalar>
void scene(CameraModel<Scalar> &camera, std::vector<bvh::Triangle<Scalar>> triangles, std::string out_file) {
    using Vector3 =  bvh::Vector3<Scalar>;
    using Bvh =  bvh::Bvh<Scalar>;
    using Triangle =  bvh::Triangle<Scalar>;

    Vector3 sun_position = Vector3(100.0, 0, 0.0);

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

    long tot_rays = 0;

#ifdef _OPENMP
    #pragma omp parallel
    {
        #pragma omp single
        std::cout << "Rendering image on " << omp_get_num_threads() << " threads..." << std::endl;
    }
#else
    std::cout << "Rendering image on single thread..." << std::endl;
#endif

    auto start2 = high_resolution_clock::now();
    auto [rays, hits] = render(camera, sun_position, bvh, triangles.data(), pixels.get());
    auto stop2 = high_resolution_clock::now();
    auto duration2 = duration_cast<microseconds>(stop2 - start2);
    tot_rays += rays;
    std::cout << "    Rays: " << rays << "\tHits: " << hits << std::endl;
    std::cout << "    Total Rays: " << tot_rays << std::endl;    
    std::cout << "    Tracing completed in " << duration2.count()/1000000.0 << " seconds\n\n";

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

void load_settings(INIReader reader, bool &use_double, std::string &output) {
    use_double = reader.GetBoolean("settings", "use_double", true);
    output = reader.Get("settings", "output", "render.png");
};

template <typename Scalar>
std::unique_ptr<CameraModel<Scalar>> load_camera(INIReader reader) {
    std::cout << "Loading camera model...\n";

    struct CameraStruct camera_struct;

    // Get the camera model:
    camera_struct.name = reader.Get("camera", "type", "PinholeCamera");

    // Get the focal length:
    camera_struct.focal_length = reader.GetReal("camera", "focal_length", 0);

    // Declare intermediate variables for parsing arrays:
    std::string segment;
    std::vector<std::string> seglist;
    std::stringstream test_str;
    int idx;

    // Get the resolution:
    auto value_str = reader.Get("camera", "resolution", "UNKNOWN");
    value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
    test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
    idx = 0;
    while(std::getline(test_str, segment, ',')) {
        camera_struct.resolution[idx] = std::stod(segment);
        idx = idx +1;
    }

    // Get the sensor size:
    value_str = reader.Get("camera", "sensor_size", "UNKNOWN");
    value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
    test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
    idx = 0;
    while(std::getline(test_str, segment, ',')) {
        camera_struct.sensor_size[idx] = std::stod(segment);
        idx = idx +1;
    }

    // Get the position:
    value_str = reader.Get("camera", "position", "[0,0,0]");
    value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
    test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
    idx = 0;
    while(std::getline(test_str, segment, ',')) {
        camera_struct.position[idx] = std::stod(segment);
        idx = idx +1;
    }

    // Get the quaternion:
    value_str = reader.Get("camera", "quaternion", "[0,0,0,1]");
    value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
    test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")-1));
    idx = 0;
    double quaternion[4];
    while(std::getline(test_str, segment, ',')) {
        quaternion[idx] = std::stod(segment);
        idx = idx +1;
    }
    quaternion[0] = 0.0;
    quaternion[1] = 0.0;
    quaternion[2] = 0.0;
    quaternion[3] = 1.0;

    std::string euler_sequence;
    double euler_angles[3];

    // Determine the rotation matrix:
    value_str = reader.Get("camera", "euler_angles", "UNKNOWN");
    std::cout << value_str.c_str() <<"\n";
    if (strcmp(value_str.c_str(),"UNKNOWN")){
        // If any euler angles are provided, use them:
        euler_sequence = reader.Get("camera", "euler_sequence", "321");
        value_str.erase(std::remove_if(value_str.begin(), value_str.end(), ::isspace), value_str.end());
        test_str = std::stringstream(value_str.substr(value_str.find("[")+1,value_str.find("]")));
        idx = 0;
        while(std::getline(test_str, segment, ',')) {
            euler_angles[idx] = std::stod(segment);
            idx = idx +1;
        }
        euler_to_rotation<double>(euler_angles, euler_sequence.c_str(), camera_struct.rotation);
    }
    else {
        // If no euler angles provided, default to the quaternion:
        quaternion_to_rotation<double>(quaternion, camera_struct.rotation);
    }

    // Print if desired:
    std::cout << "    type           : " << camera_struct.name << "\n";
    std::cout << "    focal_length   : " << camera_struct.focal_length << "\n";
    std::cout << "    resolution     : [" << camera_struct.resolution[0] << ", " << camera_struct.resolution[1] << "]\n";
    std::cout << "    sensor_size    : [" << camera_struct.sensor_size[0] << ", " << camera_struct.sensor_size[1] << "]\n";
    std::cout << "    position       : [" << camera_struct.position[0] << ", " << camera_struct.position[0] << ", " <<camera_struct.position[2] << "]\n";
    if (strcmp(value_str.c_str(),"UNKNOWN")) {
        std::cout << "    euler_sequence : " << euler_sequence[0] << "-" << euler_sequence[1] << "-" << euler_sequence[2] <<"\n";
        std::cout << "    euler_angles   : [" << euler_angles[0] << ", " << euler_angles[1] << ", " << euler_angles[2]  <<"]\n\n";
    }
    else {
        std::cout << "    quaternion     : [" << quaternion[0] << ", " << quaternion[1] << ", " << quaternion[2] << ", " << quaternion[3] <<"]\n\n";
    }

    // std::cout << "    euler_angles :" << euler_angles[0] << euler_angles[1] << euler_angles[2] << "\n";

    if (!strcmp("PinholeCamera", camera_struct.name.c_str()) ) {
        auto camera = std::make_unique<PinholeCamera<Scalar>>(camera_struct);
        return camera;
    };
    return nullptr;
};

template <typename Scalar>
std::vector<bvh::Triangle<Scalar>> load_objects(INIReader reader) {
    std::cout << "Loading .OBJs...\n";
    auto sections = reader.Sections();
    
    std::vector<bvh::Triangle<Scalar>> triangles;

    bool first_obj = true;
    for (auto it = sections.begin(); it != sections.end(); ++it) {
        if (!strcmp((*it).substr(0,3).c_str(), "obj")) {
            std::string path_to_obj = reader.Get((*it), "path", "UNKNOWN");
            if (first_obj) {
                triangles = obj::load_from_file<Scalar>(path_to_obj);
                first_obj = false;
            }
            else {
                auto triangles_new = obj::load_from_file<Scalar>(path_to_obj);
                triangles.insert(triangles.end(), triangles_new.begin(), triangles_new.end() );
            };
            std::cout << (*it).substr(4) << " loaded from " << path_to_obj << "\n";
        };
    };
    return triangles;
}

// load_lights(INIReader reader) {
//     // Get the sections from the INI file:
//     auto sections = reader.Sections();

//     std::cout << "\nREADING ALL LIGHTS\n";
//     for (auto it = sections.begin(); it != sections.end(); ++it) {
//         if (!strcmp((*it).substr(0,3).c_str(), "lgt")) {
//             std::cout << (*it).substr(4) << "\n";
//             std::cout << "   " << reader.Get((*it), "type", "UNKNOWN") << "\n";
//             std::cout << "   " << reader.Get((*it), "position", "UNKNOWN") << "\n";
//         };
//     }
// }


int main(int argc, char** argv) {

    if (argc != 2) {
        std::cout << "An INI configuration file must be provided\n";
        return 1;
    }
    else {
        std::cout << "Processing INI configuration given by: " << argv[1] << "\n";
    }

    // Things to be read from the INI:

    // Parse the INI configuration file:
    INIReader reader(argv[1]);
    
    bool use_double;
    std::string output;
    load_settings(reader, use_double, output);

    // auto lights = load_lights(reader);

    // CameraStruct camera_struct = load_camera_struct(reader);

    // Build and render the scene as double precision:
    if (use_double) {
        auto camera = load_camera<double>(reader);
        auto triangles = load_objects<double>(reader);
        scene<double>(*camera, triangles, output);
    
    // Build and render the scene as single precision:
    } else {
        auto camera = load_camera<float>(reader);
        auto triangles = load_objects<float>(reader);
        scene<float>(*camera, triangles, output);
    }
    return 0;
}