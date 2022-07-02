#ifndef __CRTB_FILE_
#define __CRTB_FILE_

#define TINYOBJLOADER_IMPLEMENTATION

#include <fstream>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <limits>

#include <zstd.h>

#include <chrono>

#include <iostream>

#include <cstring>
#include <memory>
#include <vector>
#include <random>

#include "model_loaders/tiny_obj_loader.hpp"

// Define the magic phrase at start of the file:
const char magic[] = "CRTOBJ";
const int magic_length = 6;

using Scalar = double;

typedef struct {
  std::vector<float> vertices;
  std::vector<float> normals;
  std::vector<float> texcoords;
  std::vector<int> v_indices;
  std::vector<int> vn_indices;
  std::vector<int> vt_indices;

  std::vector<tinyobj::material_t> materials;

} MyMesh;


void read_obj(std::string input_file,
              std::vector<std::vector<Scalar>> &vertices,
              std::vector<std::vector<uint32_t>> &faces){

    std::cout << "Reading OBJ file from " << input_file << "\n";
    auto start = std::chrono::system_clock::now();
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;

    if (!reader.ParseFromFile(input_file, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
    std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    // Loop over vertices:
    for (size_t i = 0; i < attrib.vertices.size(); i=i+3){
        std::vector<Scalar> vertex;
        tinyobj::real_t vx_i = attrib.vertices[size_t(i)+0];
        tinyobj::real_t vy_i = attrib.vertices[size_t(i)+1];
        tinyobj::real_t vz_i = attrib.vertices[size_t(i)+2];
        vertex.push_back((Scalar) vx_i);
        vertex.push_back((Scalar) vy_i);
        vertex.push_back((Scalar) vz_i);
        vertices.push_back(vertex);
    }

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            std::vector<uint32_t> face_def;
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx_i = shapes[s].mesh.indices[index_offset + v];
                face_def.push_back((uint32_t) idx_i.vertex_index);
            }
            faces.push_back(face_def);
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "    DONE: " << vertices.size() << " vertices and " << faces.size() << " faces read in " << elapsed_seconds.count() << " seconds\n";
}

void write_crtb(std::string file_path,
                std::vector<std::vector<Scalar>> vertices,
                std::vector<std::vector<uint32_t>> faces){

    // HEADER FORMAT:
    // {char magic[6], uint32_t compressed_v_size, uint32_t compressed_f_size, 
    //  uint32_t num_v, uint32_t num_f}
    std::cout << "Writing geometry to " << file_path << "\n";
    auto start = std::chrono::system_clock::now();

    uint32_t num_v = vertices.size();
    uint32_t num_f = faces.size();

    Scalar v_array[num_v][3];
    uint32_t f_array[num_f][3];

    // Convert vector of vector to 2d array:
    for (uint32_t i = 0; i < num_v; i++){
        for (int j = 0; j < 3; j++){
            v_array[i][j] = vertices[i][j];
        }
    }

    // Convert vector of vector to 2d array:
    for (uint32_t i = 0; i < num_f; i++){
        for (int j = 0; j < 3; j++){
            f_array[i][j] = faces[i][j];
        }
    }

    size_t size_bound;

    // Compress the vertices:
    size_t v_size = num_v*3*sizeof(Scalar);
    size_bound = ZSTD_compressBound(v_size);
    uint8_t compressed_v[size_bound];
    uint32_t compressed_v_size = ZSTD_compress(&compressed_v, size_bound, v_array, v_size, 4);
    if (ZSTD_isError(compressed_v_size)) {
        std::cerr << ZSTD_getErrorName(compressed_v_size) << std::endl;
        assert(!ZSTD_isError(compressed_v_size));
    }

    // Compress the faces:
    size_t f_size = num_f*3*sizeof(uint32_t);
    size_bound = ZSTD_compressBound(f_size);
    uint8_t compressed_f[size_bound];
    uint32_t compressed_f_size = ZSTD_compress(&compressed_f, size_bound, f_array, f_size, 4);
    if (ZSTD_isError(compressed_f_size)) {
        std::cerr << ZSTD_getErrorName(compressed_f_size) << std::endl;
        assert(!ZSTD_isError(compressed_f_size));
    }

    // Write uncompressed header to file:
    const char *file_char_ptr = file_path.c_str();
    FILE* file = fopen(file_char_ptr, "wb");
    fwrite(magic, sizeof(char), magic_length, file);
    fwrite(&compressed_v_size, sizeof(uint32_t), 1, file);
    fwrite(&compressed_f_size, sizeof(uint32_t), 1, file);
    fwrite(&num_v, sizeof(uint32_t), 1, file);
    fwrite(&num_f, sizeof(uint32_t), 1, file);

    // Write out the compressed data blocks:
    fwrite(&compressed_v, compressed_v_size, 1, file);
    fwrite(&compressed_f, compressed_f_size, 1, file);
    fclose(file);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "    DONE: " << num_v << " vertices and " << num_f << " faces written in " << elapsed_seconds.count() << " seconds\n";
}

void read_crtb(std::string file_path, 
               std::vector<std::vector<Scalar>> &vertices,
               std::vector<std::vector<uint32_t>> &faces){

    // HEADER FORMAT:
    // {char magic[6], uint32_t compressed_v_size, uint32_t compressed_f_size, 
    //  uint32_t num_v, uint32_t num_f}
    std::cout << "Reading geometry from " << file_path << "\n";
    auto start = std::chrono::system_clock::now();

    char magic_return[magic_length];
    uint32_t compressed_v_size;
    uint32_t compressed_f_size;
    uint32_t num_v;
    uint32_t num_f;

    // Read the file:
    const char *file_char_ptr = file_path.c_str();
    FILE* file = fopen(file_char_ptr, "rb");

    // Verify magic keyword:
    fread(&magic_return, sizeof(char), 6, file);
    fread(&compressed_v_size, sizeof(uint32_t), 1, file);
    fread(&compressed_f_size, sizeof(uint32_t), 1, file);
    fread(&num_v, sizeof(uint32_t), 1, file);
    fread(&num_f, sizeof(uint32_t), 1, file);

    // TODO: DEBUG WHY THIS DOESNT WORK:
    for (size_t i = 0; i < magic_length; i++){
        assert(magic_return[i] == magic[i]);
    }

    // Decompress vertices:
    uint8_t v_compressed[compressed_v_size];
    fread(&v_compressed, sizeof(uint8_t), compressed_v_size, file);
    Scalar v_array[num_v][3];
    auto size_v = ZSTD_decompress(&v_array, num_v*3*sizeof(Scalar), v_compressed, compressed_v_size);
    if (ZSTD_isError(size_v)) {
        std::cerr << compressed_v_size << " " << ZSTD_getErrorName(size_v) << std::endl;
    }

    // Decompress faces:
    uint8_t f_compressed[compressed_f_size];
    fread(&f_compressed, sizeof(uint8_t), compressed_f_size, file);
    uint32_t f_array[num_f][3];
    auto size_f = ZSTD_decompress(&f_array, num_f*3*sizeof(uint32_t), f_compressed, compressed_f_size);
    if (ZSTD_isError(size_f)) {
        std::cerr << compressed_f_size << " " << ZSTD_getErrorName(size_f) << std::endl;
    }

    fclose(file);

    // Convert 2d array to vector of vector:
    for (uint32_t i = 0; i < num_v; i++){
        std::vector<Scalar> vertex;
        for (int j = 0; j < 3; j++){
            vertex.push_back(v_array[i][j]);
        }
        vertices.push_back(vertex);
    }

    // Convert 2d array to vector of vector:
    for (uint32_t i = 0; i < num_f; i++){
        std::vector<uint32_t> face_def;
        for (int j = 0; j < 3; j++){
            face_def.push_back(f_array[i][j]);
        }
        faces.push_back(face_def);
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "    DONE: " << num_v << " vertices and " << num_f << " faces read in " << elapsed_seconds.count() << " seconds\n";
}

void print_vertices(std::vector<std::vector<Scalar>> vertices){
    for (auto vertex : vertices){
        std::cout << "v ";
        for (auto vi : vertex){
            std::cout << vi <<" ";
        }
        std::cout << "\n";
    }
}

void print_faces(std::vector<std::vector<uint32_t>> faces){
    for (auto face : faces){
        std::cout << "f ";
        for (auto idx : face){
            std::cout << idx+1 <<" ";
        }
        std::cout << "\n";
    }
}


int main(){
    std::string file = "bennu_tile.crtb";
    std::string obj_file = "bennu_tile.obj";

    // Read a .OBJ:
    std::vector<std::vector<Scalar>> v;
    std::vector<std::vector<uint32_t>> f;
    read_obj(obj_file,v,f);
    // print_vertices(v);
    // print_faces(f);
    
    // Write triangles to binary:
    write_crtb(file,v,f);

    // Read triangles from binary:
    std::vector<std::vector<uint32_t>> f2;
    std::vector<std::vector<Scalar>> v2;
    read_crtb(file, v2, f2);

    // Print triangles:
    // std::cout << "OBJ loaded from binary:\n";
    // print_vertices(v2);
    // print_faces(f2);
    
    return 0;
}

#endif