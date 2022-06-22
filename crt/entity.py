import _crt

import numpy as np

from crt._rigid_body import RigidBody

class Entity(RigidBody):
    def __init__(self,geometry_path, color=[1,1,1], geometry_type="obj", smooth_shading=False, scale=1, position=np.zeros(3), rotation=np.eye(3)):
        self.geometry_path = geometry_path
        self.geometry_type = geometry_type
        self.color = color
        self.smooth_shading = smooth_shading

        self._cpp = _crt.Entity(self.geometry_path, self.geometry_type, self.smooth_shading, self.color)
        self.set_pose(position,rotation)
        self.set_scale(scale)