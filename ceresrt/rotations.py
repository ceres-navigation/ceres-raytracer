import numpy as np

def rotation_1(rot):
    return np.array([[1,0,0],[0,np.cos(rot),np.sin(rot)],[0,-np.sin(rot),np.cos(rot)]])

def rotation_2(rot):
    return np.array([[np.cos(rot),0,-np.sin(rot)],[0,1,0],[np.sin(rot),0,np.cos(rot)]])

def rotation_3(rot):
    return np.array([[np.cos(rot),np.sin(rot),0],[-np.sin(rot),np.cos(rot),0],[0,0,1]])

def euler_to_rotmat(sequence, angles, degrees = False):
    if degrees:
        angles = np.deg2rad(angles)
    
    rotmat = np.eye(3)

    for idx, axis in enumerate(sequence):
        if axis == "1":
            new_rot = rotation_1(angles[idx])
        elif axis == "2":
            new_rot = rotation_2(angles[idx])
        elif axis == "3":
            new_rot = rotation_3(angles[idx])
        else:
            raise Exception("Not a valid euler angle sequence!")

        rotmat = np.matmul(rotmat, new_rot)

    return rotmat