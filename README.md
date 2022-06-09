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
### Build from Source (Linux/MacOS):
CRT uses `pybind11` to generate python bindings to the core C++ code.  Because of this, you must have `pybind11` installed on your machine.  We recommend using `pip`:
```
pip install pybind11
```

Once `pybind11` is installed, compiling is done via clong the repository and building using cmake and make:
```
git clone https://github.com/ceres-navigation/ceres-raytracer.git
cd ceres-raytracer
mkdir build
cd build
cmake ..
make -j
```

*NOTE: the `-j` argument to `make` allows for parallel usage of all available cores for compiling, dramatically speeding up compile times.  If you do not want a parallel compilation, you can simply run `make` with no arguments.*

### Windows:
*Coming Soon*

**NOTE:** Unfortunately, OpenMP is not properly supported by Windows and so parallel tracing is not yet supported in Windows native.  If you wish to compile from source on a Windows machine, we recommend using the [Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/about).


## Usage
### Using the python API to render a simple scene:
To run the example python file, navigate to the root directory of the cloned `ceres-raytracer` repository and simply run: 
```
cd cornell_box/
python cornell_box.py
```

Doing so will yield an image named `cornell_box_py.png` which should look like the following image:

![](cornell_box/data/cornell_box.png)

### Using the compiled C++ binary to render a simple scene:
For development purposes (to ensure that the C++ library remains functional beyond the python bindings), the compilation process also produces an executable named `ceres-rt`.  This executable needs to be given a configuration `.INI` file which defines the layout of the scene.
```
cp build/ceres-rt cornell_box/
cd cornell_box/
./ceres-rt cornell_box/cornell_box.ini
```

*NOTE: `ceres-rt` is not intended for use beyond development purposes.  Please use the python API.*


## Tasks:
- [ ] Implement physically based radiance tracking for paths
- [ ] Improve the adaptive sampling noise calculation
- [ ] Triangular meshes
  - [ ] Add vertex color support
  - [x] Add parent object support
  - [ ] Add parsers for more mesh type (.PLY, .GLTF/.GLB)
  - [ ] Add texture mapping and normal maps
- [x] Python Refactor
  - [x] Reorganize code into classes
  - [x] Move INI parser out of `main.cpp`
  - [x] Add python interface
  - [x] Animation/sequence support
- [ ] Importance Sampling
  - [ ] Implement Malley's method for cosine importance
  - [ ] Investigate alternative importance sampling method for planetary bodies (where primary indirect contribution is near horizon)
- [ ] Lighting
  - [x] Add output intensity to light objects
  - [ ] Resolve placement issue with square lights
  - [ ] Add circular area lights
  - [ ] Add emissive mesh geometries
  - [ ] Add polarized light
  - [ ] Add specific wavelength support
- [ ] Attitude (Orientation)
  - [ ] Fix euler angle sequencing issue

## Attributions
## madmann91's Modern C++ BVH Construction and Traversal Library
This project utilizes a BVH construction and traversal library built by [madmann91](https://github.com/madmann91).  While we have made some modifications to their implementation, their work forms most of the basis of the bounding volume heirarchy used in this project.  Their originaly source code can be found in the [bvh repository](https://github.com/madmann91/bvh)

## National Science Foundation Graduate Research Fellowship
This material is based upon work supported by the [National Science Foundation Graduate Research Fellowship](https://www.nsfgrfp.org/) under Grant No. 2020305048.  NSF GRFP gave Chris Gnam the flexibility and resources required to complete his research in spacecraft navigaiton, and this project is meant to serve as an open source implementation of his dissertation.

## Contact
All questionsm, comments, and concerns should be directed to Chris Gnam: crgnam@buffalo.edu
