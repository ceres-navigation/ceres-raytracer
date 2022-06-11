from crt.cameras import PinholeCamera
import numpy as np

# Default values:
focal = 30
resolution = [500,500]
sensor_size = [20,20]

position = np.array([1,2,3])

# Define pinhole camera tests:
def test_pinhole_focal():
    cam = PinholeCamera(focal, resolution, sensor_size)
    assert(cam.focal_length == focal)

def test_pinhole_resolution():
    cam = PinholeCamera(focal, resolution, sensor_size)
    assert(cam.resolution == resolution)

def test_pinhole_sensor_size():
    cam = PinholeCamera(focal, resolution, sensor_size)
    assert(cam.sensor_size == sensor_size)

def test_pinhole_set_position():
    cam = PinholeCamera(focal, resolution, sensor_size)
    cam.set_position(position)
    assert((cam.position == position).any())

def test_pinhole_position():
    cam = PinholeCamera(focal, resolution, sensor_size, position=position)
    assert((cam.position == position).any())

# Run the tests
test_pinhole_focal()
test_pinhole_resolution()
test_pinhole_sensor_size()
test_pinhole_set_position()
test_pinhole_position()