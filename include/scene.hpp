#ifndef __SCENE_H
#define __SCENE_H

template <typename Scalar>
void scene(CameraModel<Scalar> &camera, std::vector<bvh::Triangle<Scalar>> triangles, std::vector<PointLight<Scalar>> point_lights, std::string out_file) {
    using Bvh =  bvh::Bvh<Scalar>;
    using Triangle =  bvh::Triangle<Scalar>;

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
    render(camera, point_lights, bvh, triangles.data(), pixels.get());
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

#endif