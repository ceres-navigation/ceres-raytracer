import numpy as np

def sanitize_array(array):
    array_cpp = np.zeros(array.shape)
    if len(array.shape) == 1:
        for i in range(0,array.shape[0]):
            array_cpp[i] = array[i]
    elif len(array.shape) == 2:
        for i in range(0,array.shape[0]):
            for j in range(0,array.shape[1]):
                array_cpp[i,j] = array[i,j]
    elif len(array.shape) == 3:
        for i in range(0,array.shape[0]):
            for j in range(0,array.shape[1]):
                for k in range(0,array.shape[2]):
                    array_cpp[i,j,k] = array[i,j,k]
    return array_cpp

def validate_scale(scale: float):
    err_msg = "Scale must be defined as a single positive float/int"
    assert((type(scale) == float) or (type(scale) == int), err_msg)
    assert((scale > 0), err_msg)
    return scale

def validate_position(position):
    err_msg = """Positions must be defined as:\n
                    - List of floats/ints of length 3\n
                    - Tuple of floats/ints of length 3\n
                    - Ndarray of floats/ints with size 3"""

    if (type(position) == list) or (type(position) == tuple):
        assert( len(position) == 3, err_msg)
        for index in position:
            assert((type(index) == float) or (type(index) == int), err_msg)
        position = np.array(position)
    elif (type(position) == np.ndarray):
        position = position.squeeze()
        assert(position.size == 3, err_msg)

    position = sanitize_array(position)

    return position


def validate_rotation(rotation):
    err_msg = """Rotations must be defined as ndarray with shape (3,3).  Additionally,\n
                 rotation matrices must also have a determinant of 1, and their inverse\n
                 must be equal to their transpose."""

    tol = 1e-9
    assert(type(rotation) == np.ndarray, err_msg)
    assert(rotation.shape == (3,3), err_msg)
    assert(np.linalg.det(rotation) - 1 < tol, err_msg)
    assert(np.linalg.norm(np.matmul(rotation.T,rotation) - np.eye(3)) < tol, err_msg)

    rotation = sanitize_array(rotation)
    
    return rotation
