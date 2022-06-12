from crt.lights import PointLight, SquareLight
import numpy as np

# Default values:
intensity = 10
size = [3,5]

position1 = np.array([1,2,3])
position2 = np.array([3,2,1])
rotation1 = np.array([[0,-1,0],[0,0,-1],[1,0,0]])
rotation2 = rotation1.T

# Define PointLight tests:
plight = PointLight(intensity, position=position1)
def test_plight_intensity():
    assert(plight.intensity == intensity)
# def test_plight_position():
#     assert((plight.position == position1).all())
def test_plight_set_position():
    plight.set_position(position2)
    assert((plight.position == position2).all())

# Define SquareLight tests:
sqlight = SquareLight(intensity, size, position=position1, rotation=rotation1)
def test_sqlight_intensity():
    assert(sqlight.intensity == intensity)
def test_sqlight_size():
    assert(sqlight.size == size)
def test_sqlight_position():
    assert((sqlight.position == position1).all())
def test_sqlight_rotation():
    assert((sqlight.rotation == rotation1).all())

def test_sqlight_set_position():
    sqlight.set_position(position2)
    assert((sqlight.position == position2).all())
def test_sqlight_set_rotation():
    sqlight.set_rotation(rotation2)
    assert((sqlight.rotation == rotation2).all())
def test_sqlight_set_pose():
    sqlight.set_pose(position1,rotation1)
    assert((sqlight.rotation == rotation1).all() and (sqlight.position == position1).all())

# Run the tests
test_plight_intensity()
# test_plight_position()
test_plight_set_position()

test_sqlight_intensity()
test_sqlight_size()
test_sqlight_position()
test_sqlight_rotation()
test_sqlight_set_position()
test_sqlight_set_rotation()
test_sqlight_set_pose()