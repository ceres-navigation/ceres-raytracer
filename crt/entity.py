import _crt

import numpy as np
from numpy.typing import ArrayLike

from crt.rigid_body import RigidBody

class Entity(RigidBody):
    """
    The :class:`Entity` class stores all mesh, material, and texture information for a body.

    :param geometry_path: Path to the mesh geometry to be loaded
    :type geometry_path: str
    :param color: The RGB color code of the geometry |default| :code:`[1,1,1]`
    :type color: ArrayLike, optional
    :param geometry_type: The format of the mesh geoemtry provided |default| :code:`"obj"`
    :type geometry_type: str, optional
    :param smooth_shading: Flag to enable smooth shading via vertex normal interpolation |default| :code:`False`
    :type smooth_shading: bool, optional
    """
    def __init__(self,geometry_path: str, color: ArrayLike =[1,1,1], geometry_type: str="obj", 
                 smooth_shading: bool=False, **kwargs):
        super(Entity, self).__init__(**kwargs)

        self.geometry_path = geometry_path
        """
        Path to the geometry (:code:`str`)
        """

        self.geometry_type = geometry_type
        """
        Format of the provided geometry file (:code:`str`)
        """

        self.color = color
        """
        RGB Color code for the geometry (:code:`ArrayLike`)
        """

        self.smooth_shading = smooth_shading
        """
        Flag to enable smooth shading via vertex normal interpolation (:code:`bool`)
        """

        self._cpp = _crt.Entity(self.geometry_path, self.geometry_type, self.smooth_shading, self.color)
        """
        Corresponding C++ Entity object
        """

        self.set_pose(self.position,self.rotation)
        self.set_scale(self.scale)