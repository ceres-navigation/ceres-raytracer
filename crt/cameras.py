import numpy as np
from abc import ABC, abstractmethod

import _crt
from crt.rigid_body import RigidBody
from numpy.typing import ArrayLike

class Camera(ABC):
    """
    The :class:`Camera` abstract base class 
    """
    @abstractmethod
    def get_fov(self, degrees=True):
        return


class SimpleCamera(RigidBody, Camera):
    """
    The :class:`SimpleCamera` class is the simplest camera model implemented

    :param focal_length: Focal length of the camera
    :type focal_length: float
    :param resolution: Resolution of camera
    :type resolution: ArrayLike
    :param sensor_size: Size of the camera's sensor
    :type sensor_size: ArrayLike
    :param z_positive: Flag for if the camera's boresight is aligned with positive z-axis |default| :code:`False`
    :type z_positive: bool, optional
    """
    def __init__(self, focal_length: float, resolution: ArrayLike, sensor_size: ArrayLike, 
                 z_positive: bool=False, **kwargs):
        
        super(SimpleCamera, self).__init__(**kwargs)

        self.focal_length = focal_length
        """
        Focal length of the camera (:code:`float`)
        """

        self.resolution   = resolution
        """
        Resolution of the camera (:code:`numpy.array` of shape :code:`(2,)`)
        """

        self.sensor_size  = sensor_size
        """
        Sensor size of the camera (:code:`numpy.array` of shape :code:`(2,)`)
        """

        self._cpp = _crt.SimpleCamera(focal_length, resolution, sensor_size,z_positive)
        """
        Corresponding C++ SimpleCamera object
        """

        self.set_pose(self.position, self.rotation)

    def get_fov(self, degrees: bool=True) -> np.ndarray:
        """
        Calculate and return the angular field of view of the defined camera model

        :param degrees: Flag for if the returned field of view has units of degrees |default| :code:`True`
        :type degrees: bool, optional
        :return: The calculated angular field of view(s)
        :rtype: np.ndarray
        """
        if degrees:
            return np.rad2deg(2*np.arctan2(self.sensor_size/2, 2*self.focal_length))
        else:
            return 2*np.arctan2(self.sensor_size/2, 2*self.focal_length)
    
        