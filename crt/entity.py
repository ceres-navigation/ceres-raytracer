import _crt

import numpy as np

from crt._sanitize import sanitize_array as SA

class Entity:
    def __init__(self,geometry_path, color=[1,1,1], geometry_type="obj", smooth_shading=False, scale=1, position=np.zeros(3), rotation=np.eye(3)):
        self.geometry_path = geometry_path
        self.geometry_type = geometry_type
        self.color = color
        self.smooth_shading = smooth_shading
        self.scale = scale
        self.position = position
        self.rotation = rotation

        self._cpp = _crt.Entity(self.geometry_path, self.geometry_type, self.smooth_shading, self.color)
        self._cpp.set_pose(SA(self.position), SA(self.rotation))
        self._cpp.set_scale(self.scale)

    def set_scale(self, scale):
        self.scale = scale
        self._cpp.set_scale(self.scale)

    def set_position(self, position):
        self.position = position
        self._cpp.set_position(SA(self.position))

    def set_rotation(self, rotation):
        self.rotation = rotation
        self._cpp.set_rotation(SA(self.rotation))

    def set_pose(self, position, rotation):
        self.position = position
        self.rotation = rotation
        self._cpp.set_pose(SA(self.position), SA(self.rotation))