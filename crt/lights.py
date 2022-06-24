import numpy as np
from abc import ABC

from numpy.typing import ArrayLike

import _crt
from crt.rigid_body import RigidBody

class Light(ABC):
    """
    The :class:`Light` abstract base class
    """

    intensity: float
    """
    Intensity of the light source (:code:`float`)
    """

class PointLight(RigidBody, Light):
    """
    The :class:`PointLight` class is the simplest light model implemented

    :param intensity: Intensity of the light source
    :type intensity: float
    """
    def __init__(self, intensity: float, **kwargs):
        super(PointLight, self).__init__(intensity, **kwargs)

        self._cpp = _crt.PointLight(self.intensity)
        """
        Corresponding C++ PointLight object
        """

        self.set_pose(self.position, np.eye(3))

class SquareLight(RigidBody, Light):
    """
    The :class:`SquareLight` class is the simplest area light model implemented

    :param intensity: Intensity of the light source
    :type intensity: float
    :param size: Lengths of the sides of the light
    :type size: ArrayLike
    """
    def __init__(self, intensity: float, size: ArrayLike, **kwargs):
        super(SquareLight, self).__init__(intensity, **kwargs)

        self.size = size
        """
        Lengths of the sides of the light (:code:`numpy.ndarray` or shape :code:`(2,)`)
        """

        self._cpp = _crt.SquareLight(self.intensity, self.size)
        """
        Corresponding C++ SquareLight object
        """

        self.set_pose(self.position, self.rotation)