# CERES Ray Tracer


## Installation
**Dependencies:**
- ImageMagick version 7 \*

\* *NOTE: ImageMagick version 7 is not available in the standard Ubuntu repository.  If using Ubuntu, please [compile from source](https://techpiezo.com/linux/install-imagemagick-in-ubuntu-20-04-lts/?fbclid=IwAR2hNrUM9hzWnNpgkxlSfit2x1CHfmSO1hW5hNPpzcgzhcWFhsBXg4jz0Pc)*

### Linux
- `mkdir build; cd build; cmake ..`
- `make`

## Examples
First build the project using the steps above
- Simple OBJ Preview: `./render ../data/bunny.obj --eye 0 .1 -.3 --rotate y -145`

*NOTE: The output by default is `.ppm` format.  To convert to `.png` we recommend using ImageMagick*
- `sudo apt install imagemagick`
- `convert render.ppm render.png`