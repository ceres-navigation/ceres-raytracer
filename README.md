# CERES Ray Tracer (CRT)
<!-- ![Tests](https://github.com/ceres-navigation/ceres/actions/workflows/tests.yml/badge.svg) -->
<!-- [![codecov](https://codecov.io/gh/ceres-navigation/ceres/branch/main/graph/badge.svg?token=BX07Q0PITB)](https://codecov.io/gh/ceres-navigation/ceres) -->
[![GitHub issues](https://img.shields.io/github/issues/ceres-navigation/ceres-raytracer)](https://github.com/ceres-navigation/ceres-pathtracer/issues)
[![GitHub Release](https://img.shields.io/github/v/release/ceres-navigation/ceres-raytracer?include_prereleases)](https://github.com/ceres-navigation/ceres-pathtracer/releases)
[![GitHub Contributers](https://img.shields.io/github/contributors/ceres-navigation/ceres-raytracer)](https://github.com/ceres-navigation/ceres-raytracer/graphs/contributors)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
<!-- ![Mac OS](https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=macos&logoColor=F0F0F0) -->
<!-- ![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white) -->

This proejct was developed for [CERES](https://ceresnavigation.org) and aims to provide scientifically useful path tracing capabilities for:
- Rendering photo-real images
- Simulating multi-bounce behavior accounting for wavelength and polarization
- Modeling solar radiation and albedo radiation pressure on spacecraft

## Installation
**Dependencies:**
- ImageMagick version 7

### Linux/MacOS:
**If using Ubuntu** please [compile from source](https://techpiezo.com/linux/install-imagemagick-in-ubuntu-20-04-lts/?fbclid=IwAR2hNrUM9hzWnNpgkxlSfit2x1CHfmSO1hW5hNPpzcgzhcWFhsBXg4jz0Pc)

- `mkdir build; cd build; cmake ..`
- `make`

### Windows
*Coming Soon*

## INI Examples
*NOTE: All of these examples assume that `render` is run from within the build directory.*

### Cornell Box
- Simple "Cornell Box" scene" : `./render ../examples/cornell_box.ini`
![](examples/cornell_box.png)

# Tasks:
- [ ] Implement physically based radiance tracking for paths
- [ ] Improve the adaptive sampling noise calculation
- [ ] Triangular meshes
  - [ ] Add vertex color support
  - [ ] Add parent object support
  - [ ] Add parsers for more mesh type (.PLY, .GLTF/.GLB)
  - [ ] Add texture mapping and normal maps
- [ ] Refactor
  - [ ] Reorganize code into classes
  - [ ] Move INI parser out of `main.cpp`
  - [ ] Add python interface
  - [ ] Animation/sequence support
- [ ] Importance Sampling
  - [ ] Implement Malley's method for cosine importance
  - [ ] Investigate alternative importance sampling method for planetary bodies (where primary indirect contribution is near horizon)
- [ ] Lighting
  - [ ] Add output intensity to light objects
  - [ ] Resolve placement issue with square lights
  - [ ] Add circular area lights
  - [ ] Add emissive mesh geometries
  - [ ] Add polarized light
  - [ ] Add specific wavelength support
- [ ] Attitude (Orientation)
  - [ ] Fix euler angle sequencing issue

# Attributions
## madmann91's Modern C++ BVH Construction and Traversal Library
This project utilizes a BVH construction and traversal library built by [madmann91](https://github.com/madmann91).  While we have made some modifications to their implementation, their work forms most of the basis of the bounding volume heirarchy used in this project.  Their originaly source code can be found in the [bvh repository](https://github.com/madmann91/bvh)

## National Science Foundation Graduate Research Fellowship
This material is based upon work supported by the [National Science Foundation Graduate Research Fellowship](https://www.nsfgrfp.org/) under Grant No. 2020305048.  NSF GRFP gave Chris Gnam the flexibility and resources required to complete his research in spacecraft navigaiton, and this project is meant to serve as an open source implementation of his dissertation.

# Contact
All questionsm, comments, and concerns should be directed to Chris Gnam: crgnam@buffalo.edu
