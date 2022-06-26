import numpy as np
from abc import ABC, abstractmethod

import _crt
from crt.rigid_body import RigidBody
from numpy.typing import ArrayLike

class Lidar(ABC):
    """
    The :class:`Camera` abstract base class 
    """
    @abstractmethod
    def get_fov(self, degrees=True):
        return


class SimpleLidar(RigidBody, Lidar):
    """
    The :class:`SimpleLidar` class is the simplest lidar model implemented.  It casts
    a single ray in teh direction the lidar is currently pointed.

    :param z_positive: Flag for if the camera's boresight is aligned with positive z-axis |default| :code:`False`
    :type z_positive: bool, optional
    """
    def __init__(self, z_positive: bool=False, **kwargs):
        super(SimpleLidar, self).__init__(**kwargs)

        self._cpp = _crt.SimpleLidar(z_positive)
        """
        Corresponding C++ SimpleCamera object
        """

        self.set_pose(self.position, self.rotation)