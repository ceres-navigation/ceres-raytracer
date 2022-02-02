# CERES Ray Tracer


## Installation
### Linux
- `mkdir build; cd build; cmake ..`
- `make`

## Examples
First build the project using the steps above
- Simple OBJ Preview: `./render ../data/bunny.obj --eye 0 .1 -.3 --rotate y -145`

*NOTE: The output by default is `.ppm` format.  To convert to `.png` we recommend using ImageMagick*
- `sudo apt install imagemagick`
- `convert render.ppm render.png`