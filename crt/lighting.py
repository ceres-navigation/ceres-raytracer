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
    def __init__():
        return