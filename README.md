# CERES Ray Tracer (CRT)
[![PyPI version](https://img.shields.io/pypi/v/ceres-raytracer)](https://pypi.org/project/ceres-raytracer/)
[![GitHub Release](https://img.shields.io/github/v/release/ceres-navigation/ceres-raytracer?include_prereleases)](https://github.com/ceres-navigation/ceres-raytracer/releases)
[![GitHub issues](https://img.shields.io/github/issues/ceres-navigation/ceres-raytracer)](https://github.com/ceres-navigation/ceres-raytracer/issues)
[![GitHub Contributers](https://img.shields.io/github/contributors/ceres-navigation/ceres-raytracer)](https://github.com/ceres-navigation/ceres-raytracer/graphs/contributors)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Size](https://img.shields.io/github/repo-size/ceres-navigation/ceres-raytracer)

This project was developed as part of [CERES](https://ceresnavigation.org) and aims to provide scientifically useful path tracing capabilities for rendering images, simulating LiDAR, modeling radiation pressure, and flux computations for thermal and solar panel analysis.

- [Website](https://ceresnavigation.org)
- [API Documentation](https://docs.crt.ceresnavigation.org)
- [Source code](https://github.com/ceres-navigation/ceres-raytracer)
- [Bug Reports](https://github.com/ceres-navigation/ceres-raytracer/issues)


### Available Environments
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
<!-- ![Mac OS](https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=macos&logoColor=F0F0F0) -->
<!-- ![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white) -->

[![](https://codecov.io/gh/ceres-navigation/ceres-raytracer/branch/main/graph/badge.svg?token=BX07Q0PITB)](https://app.codecov.io/gh/ceres-navigation/ceres-raytracer)
| Environment   | Build Python Binding | Install from PyPI |
| ------------- |:--------------------:|:-----------------:|
| Ubuntu 22.04  | [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/build_ubuntu22.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions) <br /> [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/test_ubuntu22.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions) | [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/pypi_ubuntu22.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions) <br /> [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/pypi_test_ubuntu22.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions)
| Ubuntu 20.04  | [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/build_ubuntu20.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions) <br /> [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/test_ubuntu20.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions) | [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/pypi_ubuntu20.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions) <br /> [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/pypi_test_ubuntu20.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions)
| Ubuntu 18.04  | [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/build_ubuntu18.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions) <br /> [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/test_ubuntu18.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions) | [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/pypi_ubuntu18.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions) <br /> [![](https://github.com/ceres-navigation/ceres-raytracer/actions/workflows/pypi_test_ubuntu18.yml/badge.svg)](https://github.com/ceres-navigation/ceres-raytracer/actions)

***
## Installation:
It is recommended to install directly from the Python Package Index (PyPI):
```
pip install ceres-raytracer
```

***
## Building from Source:

```
git clone https://github.com/ceres-navigation/ceres-raytracer.git
pip install .
```

[sphinx](https://www.sphinx-doc.org/en/master/) is used to build documentation for the python API.  To install `sphinx`, simply use:
```
sudo apt-get install python3-sphinx
pip install sphinx-autodoc-typehints sphinx-toolbox sphinx-licenseinfo sphinx-rtd-theme
```
Once `sphinx` is installed, you can build the documentation on Linux systems by running:
```
cd doc
make html
```

***
## Demos:
After installing `ceres-raytracer`, simply clone the [ceres-raytracer-demos](https://github.com/ceres-navigation/ceres-raytracer-demos):
```
git clone https://github.com/ceres-navigation/ceres-raytracer-demos.git
```
and run any of the included `.py` example files:

### cornel_box.py
This demo script demonstrates multi-bounce path tracing using the traditional cornell box scene.  It additionally shows how to render depth, instances and normal passes:

![](https://raw.githubusercontent.com/ceres-navigation/ceres-raytracer-demos/master/results/cornell.png)

### comet67p.py
This demo script demonstrates how to use CRT's SPICE support, and rendering in a body fixed frame (allowing for the BVH to be cached and therefore overall faster render times):

![](https://raw.githubusercontent.com/ceres-navigation/ceres-raytracer-demos/master/results/comet67p.gif) ![](https://raw.githubusercontent.com/ceres-navigation/ceres-raytracer-demos/master/results/lidar.png)


***
## Tasks:
- [ ] Python Interface
  - [x] Python bindings with pybind11
  - [ ] Binary distributions (cross compiled for Windows/MacOS on Linux)
  - [ ] Python defined materials (compatible only with wavefront ray tracing)
- [ ] Path tracing implementations
  - [x] Adaptive sampling
  - [ ] Wavefront ray tracing
  - [ ] Bidirectional path tracing
  - [ ] Primary Sample Space Metropolis Light Transport
- [ ] Physics
  - [ ] Polarization of light rays
  - [ ] Wavelength of light rays
  - [ ] Spectral power density of rays
  - [ ] Physically based radiance tracking for paths
  - [ ] Momentum of paths
- [ ] Materials
  - [ ] PBR Texture support
  - [ ] Hapke function support
  - [ ] Microfacet support
- [ ] Major architecture changes
  - [ ] TBB for parallelization
  - [ ] Add Embree single precision support
  - [ ] Add OptiX (GPU) single precision support
  - [ ] Add Vulkan (GPU) single precision support

***
## Attributions
### madmann91's Modern C++ BVH Construction and Traversal Library
This project utilizes a BVH construction and traversal library built by [madmann91](https://github.com/madmann91).  While we have made some modifications to their implementation, their work forms most of the basis of the bounding volume hierarchy used in this project.  Their originally source code can be found in the [bvh repository](https://github.com/madmann91/bvh)


### National Science Foundation Graduate Research Fellowship
This material is based upon work supported by the [National Science Foundation Graduate Research Fellowship](https://www.nsfgrfp.org/) under Grant No. 2020305048.  NSF GRFP gave Chris Gnam the flexibility and resources required to complete his research in spacecraft navigation, and this project is meant to serve as an open source implementation of his dissertation.

***
## Contact
All questions, comments, and concerns should be directed to Chris Gnam: crgnam@buffalo.edu
