import _crt
import numpy as np

class PointLight:
    def __init__(self, intensity, position=np.zeros(3)):
        self.intensity = intensity
        self.position = position
        self._cpp = _crt.PointLight(self.intensity)
        self._cpp.set_position(self.position)

    def set_position(self, position):
        self.position = position
        self._cpp.set_position(self.position)

class SquareLight:
    def __init__(self, intensity, size, position=np.zeros(3),rotation=np.eye(3)):
        self.intensity = intensity
        self.size = size
        self.position = position
        self.rotation = rotation
        self._cpp = _crt.SquareLight(self.intensity, self.size)
        self._cpp.set_pose(self.position, self.rotation)

    def set_position(self, position):
        self.position = position
        self._cpp.set_position(self.position)

    def set_rotation(self, rotation):
        self.rotation = rotation
        self._cpp.set_rotation(self.rotation)

    def set_pose(self, position, rotation):
        self.position = position
        self.rotation = rotation
        self._cpp.set_pose(self.position, self.rotation)