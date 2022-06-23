import _crt

import numpy as np

from crt.rigid_body import RigidBody

class Entity(RigidBody):
    def __init__(self,geometry_path, color=[1,1,1], geometry_type="obj", smooth_shading=False, **kwargs):
        super(Entity, self).__init__(**kwargs)

        self.geometry_path = geometry_path
        self.geometry_type = geometry_type
        self.color = color
        self.smooth_shading = smooth_shading

        # Create corresponding C++ object:
        self._cpp = _crt.Entity(self.geometry_path, self.geometry_type, self.smooth_shading, self.color)
        self.set_pose(self.position,self.rotation)
        self.set_scale(self.scale)