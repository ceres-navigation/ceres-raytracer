from crt.cameras import PinholeCamera
import numpy as np

# Default values:
focal = 30
resolution = [500,500]
sensor_size = [20,20]

position1 = np.array([1,2,3])
position2 = np.array([3,2,1])
rotation1 = np.array([[0,-1,0],[0,0,-1],[1,0,0]])
rotation2 = rotation1.T

# Define pinhole phcamera tests:
phcam  = PinholeCamera(focal, resolution, sensor_size, position=position1, rotation=rotation1)
def test_pinhole_focal():
    assert(phcam.focal_length == focal)
def test_pinhole_resolution():
    assert(phcam.resolution == resolution)
def test_pinhole_sensor_size():
    assert(phcam.sensor_size == sensor_size)
def test_pinhole_position():
    assert((phcam.position == position1).all())
def test_pinhole_rotation():
    assert((phcam.rotation == rotation1).all())

def test_pinhole_set_position():
    phcam.set_position(position2)
    assert((phcam.position == position2).all())
def test_pinhole_set_rotation():
    phcam.set_rotation(rotation2)
    assert((phcam.rotation == rotation2).all())
def test_pinhole_set_pose():
    phcam.set_pose(position1,rotation1)
    assert((phcam.rotation == rotation1).all() and (phcam.position == position1).all())

# Run the tests
test_pinhole_focal()
test_pinhole_resolution()
test_pinhole_sensor_size()
test_pinhole_position()
test_pinhole_rotation()
test_pinhole_set_position()
test_pinhole_set_rotation()
test_pinhole_set_pose()
