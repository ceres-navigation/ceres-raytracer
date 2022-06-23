import numpy as np
from abc import ABC

import _crt
from crt.rigid_body import RigidBody

class PointLight(RigidBody):
    def __init__(self, intensity, **kwargs):
        super(PointLight, self).__init__(**kwargs)

        self.intensity = intensity

        # Create corresponding C++ object:
        self._cpp = _crt.PointLight(self.intensity)
        self.set_pose(self.position, np.eye(3))

class SquareLight(RigidBody):
    def __init__(self, intensity, size, **kwargs):
        super(SquareLight, self).__init__(**kwargs)

        self.intensity = intensity
        self.size = size

        # Create corresponding C++ object:
        self._cpp = _crt.SquareLight(self.intensity, self.size)
        self.set_pose(self.position, self.rotation)