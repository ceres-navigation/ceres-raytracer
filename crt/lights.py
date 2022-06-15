import _crt
import numpy as np

from crt._sanitize import sanitize_array as SA

class PointLight:
    def __init__(self, intensity, position=np.zeros(3)):
        self.intensity = intensity
        self.position = position
        self.rotation = np.eye(3)
        self._cpp = _crt.PointLight(self.intensity)
        self._cpp.set_position(SA(self.position))

    def set_position(self, position):
        self.position = position
        self._cpp.set_position(SA(self.position))

    def set_rotation(self, rotation):
        self.rotation = rotation
        self._cpp.set_rotation(SA(self.rotation))
    
    def set_pose(self, position, rotation):
        self.set_position(SA(position))
        self.set_rotation(SA(rotation))

class SquareLight:
    def __init__(self, intensity, size, position=np.zeros(3),rotation=np.eye(3)):
        self.intensity = intensity
        self.size = size
        self.position = position
        self.rotation = rotation
        self._cpp = _crt.SquareLight(self.intensity, self.size)
        self._cpp.set_pose(SA(self.position), SA(self.rotation))

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