import _crt

from abc import ABC
import numpy as np


class PinholeCamera:
    def __init__(self, focal_length, resolution, sensor_size, position=np.zeros(3), rotation=np.eye(3)):
        self.focal_length = focal_length
        self.resolution = resolution
        self.sensor_size = sensor_size
        self.position = position
        self.rotation = rotation

        self._cpp = _crt.PinholeCamera(focal_length, resolution, sensor_size)
        self._cpp.set_pose(self.position, self.rotation)

    def set_position(self, position):
        self.position = position
        self._cpp.set_position(position)

    def set_rotation(self, rotation):
        self.rotation = rotation
        self._cpp.set_rotation(rotation)

    def set_pose(self, position, rotation):
        self.position = position
        self.rotation = rotation
        self._cpp.set_pose(position,rotation)
    
        