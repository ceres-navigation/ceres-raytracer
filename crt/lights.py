import numpy as np
from abc import ABC

import _crt
from crt._rigid_body import RigidBody

class PointLight(RigidBody):
    def __init__(self, intensity, position=np.zeros(3)):
        self.intensity = intensity
        self._cpp = _crt.PointLight(self.intensity)
        self.set_pose(position, np.eye(3))

class SquareLight(RigidBody):
    def __init__(self, intensity, size, position=np.zeros(3),rotation=np.eye(3)):
        self.intensity = intensity
        self.size = size

        self._cpp = _crt.SquareLight(self.intensity, self.size)
        self.set_pose(position, rotation)