import numpy as np
from typing import Union, List, Tuple

from crt._validate_values import validate_scale, validate_position, validate_rotation

class RigidBody:
    def __init__():
        pass
    
    def set_scale(self, scale: float, cpp: bool =True):
        self.scale = validate_scale(scale)
        if cpp:
            self._cpp.set_scale(self.scale)

    def set_position(self, position: Union[np.ndarray, List[float], Tuple[float,float,float]], cpp: bool =True):
        self.position = validate_position(position)
        if cpp:
            self._cpp.set_position(self.position)

    def set_rotation(self, rotation, cpp: bool =True):
        self.rotation = validate_rotation(rotation)
        if cpp:
            self._cpp.set_rotation(self.rotation)

    def set_pose(self, position: Union[np.ndarray, List[float], Tuple[float,float,float]], rotation: np.ndarray, cpp: bool =True):
        self.position = validate_position(position)
        self.rotation = validate_rotation(rotation)
        if cpp:
            self._cpp.set_pose(self.position, self.rotation)