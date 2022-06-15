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