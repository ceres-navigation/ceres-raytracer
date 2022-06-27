import numpy as np
from abc import ABC

import _crt
from crt.rigid_body import RigidBody

from spiceypy import spkpos, pxform

from numpy.typing import ArrayLike

from crt._validate_values import validate_position, validate_rotation

class Lidar(ABC):
    """
    The :class:`Lidar` abstract base class 
    """
    def batch_set_pose(self, batch_positions: ArrayLike, batch_rotations: ArrayLike):
        self.batch_positions = np.zeros(batch_positions.shape)
        for idx in range(0, batch_positions.shape[0]):
            self.batch_positions[idx,:] = validate_position(batch_positions[idx,:])
        self.batch_rotations = batch_rotations

        # Ensure that less than 100,000 have been provided:
        assert(batch_positions.shape[0] <= 100000, 'No more than 100,000 poses can be batch set')

        batch_rotations_pad = np.zeros((3,3,100000))
        for idx in range(0, batch_rotations.shape[2]):
            batch_rotations_pad[:,:,idx] = validate_rotation(batch_rotations[:,:,idx])

        self._cpp.batch_set_pose(batch_positions, batch_rotations_pad, batch_positions.shape[0])

    def batch_spice_pose(self, ets: np.ndarray):
        positions,_ = spkpos(self.name, ets, self.ref, self.abcorr, self.origin)
        rotations = np.zeros((3,3,ets.size))
        for idx, et in enumerate(ets):
            rotation = pxform(self.ref, self.frame, et)
            rotations[:,:,idx] = validate_rotation(rotation)

        positions = positions
        new_positions = np.zeros(positions.shape)
        for idx in range(0, positions.shape[0]):
            new_positions[idx,:] = positions[idx,:]

        self.batch_set_pose(new_positions, rotations)

class SimpleLidar(RigidBody, Lidar):
    """
    The :class:`SimpleLidar` class is the simplest lidar model implemented.  It casts
    a single ray in teh direction the lidar is currently pointed.

    :param z_positive: Flag for if the Lidar's boresight is aligned with positive z-axis |default| :code:`False`
    :type z_positive: bool, optional
    """
    def __init__(self, z_positive: bool=False, **kwargs):
        super(SimpleLidar, self).__init__(**kwargs)

        self._cpp = _crt.SimpleLidar(z_positive)
        """
        Corresponding C++ SimpleLidar object
        """

        self.set_pose(self.position, self.rotation)