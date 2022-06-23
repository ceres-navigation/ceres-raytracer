import numpy as np
from typing import Union, List, Tuple
from warnings import warn

from spiceypy import spkpos, pxform

from crt._validate_values import validate_scale, validate_position, validate_rotation

class RigidBody:
    def __init__(self, position=np.zeros(3), rotation=np.eye(3), scale=1, name=None, frame=None, origin=None, ref="J2000", abcorr="NONE"):
        self.position = validate_position(position)
        self.rotation = validate_rotation(rotation)
        self.scale = validate_scale(scale)

        self.name = name
        self.frame = frame
        self.origin = origin
        self.ref = ref
        self.abcorr = abcorr

        if (self.name is not None) and (self.origin is None):
            warn("NAME was set to {} while ORIGIN is None".format(self.name.upper()))
        if (self.origin is not None) and (self.name is None):
            warn("ORIGIN was set to {} while NAME is None".format(self.name.upper()))

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

    def spice_position(self, et, cpp: bool =True):
        position,_ = spkpos(self.name, et, self.ref, self.abcorr, self.origin)
        self.position = validate_position(position)
        if cpp:
            self._cpp.set_position(self.position)

    def spice_rotation(self, et, cpp: bool =True):
        rotation = pxform(self.ref, self.frame, et)
        self.rotation = validate_rotation(rotation)
        if cpp:
            self._cpp.set_position(self.rotation)

    def spice_pose(self, et, cpp: bool =True):
        position,_ = spkpos(self.name, et, self.ref, self.abcorr, self.origin)
        rotation = pxform(self.ref, self.frame, et)
        self.position = validate_position(position)
        self.rotation = validate_rotation(rotation)
        if cpp:
            self._cpp.set_pose(self.position, self.rotation)