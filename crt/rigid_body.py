import numpy as np
from warnings import warn

from typing import Union, List, Tuple
from numpy.typing import ArrayLike

from spiceypy import spkpos, pxform

from crt._validate_values import validate_scale, validate_position, validate_rotation

class RigidBody:
    """
    The :class:`RigidBody` class is the superclass to :class:`Entity` as well as all :class:`~.cameras.Camera` and
    :class:`~.lights.Light` classes.  It provides methods for adjusting the pose (position and orientation)
    as well as scale of a body.  Additionally, it provides a SPICE interface allowing for NAIF ephemerides to be
    used to control the pose of a body.

    :param position: Position of the body |default| :code:`np.zeros(3)`
    :type position: ArrayLike, optional
    :param rotation: Rotation (orientation) of the body as a 3-by-3 rotation matrix 
                     that must obey both :math:`R^T = R^{-1}` and :math:`\det(R)=1`
                     |default| :code:`np.eye(3)`
    :type rotation: ArrayLike, optional
    :param scale: Scale of the body |default| :code:`1`
    :type scale: Union[float, int], optional
    :param name: NAIF name or ID code of body |default| :code:`None`
    :type name: Union[str, None], optional
    :param frame: NAIF reference frame name of body |default| :code:`None`
    :type frame: Union[str, None], optional
    :param origin: NAIF name or ID code of reference frame origin |default| :code:`"SSB"`
    :type origin: Union[str, None], optional
    :param ref: NAIF reference frame name of reference frame |default| :code:`"J2000"`
    :type ref: Union[str, None], optional
    :param abcorr: Aberration correction flag |default| :code:`"NONE"`
    :type abcorr: Union[str, None], optional
    """

    def __init__(self, position: ArrayLike=np.zeros(3), 
                 rotation: ArrayLike=np.eye(3), scale: Union[float, int]=1, 
                 name: Union[str, None]=None, frame: Union[str, None]=None, origin: str="SSB", 
                 ref: Union[str, None]="J2000", abcorr: Union[str, None]="NONE"):

        self.scale = validate_scale(scale)
        """
        Scale of the body (:code:`float`)
        
        This attribute is used by the subclass :class:`~.Entity` and describes how the :class:`~.Entity` 
        geometry is scaled.  It can be modified directly using :meth:`set_scale`.
        """

        self.position = validate_position(position)
        """
        Position of the body (:code:`numpy.ndarray` of shape :code:`(3,)`)
        
        This attribute is used by the subclass :class:`~.Entity`, as well as all :class:`~.cameras.Camera` and 
        :class:`~.lights.Light` classes.  It describes how the body is translated in space.  It can be modified directly 
        using either :meth:`set_position` or :meth:`set_pose`.  Alternatively, it can be modified using 
        :meth:`spice_position` which uses predefined SPICE ephemerides to calculate a position for a 
        given ephemeris time.
        """

        self.rotation = validate_rotation(rotation)
        """
        Rotation of the body (:code:`numpy.ndarray` of shape :code:`(3,3)`)
        
        This attribute is used by the subclasses :class:`~.Entity`, as well as all :class:`~.cameras.Camera` and 
        :class:`~.lights.Light` classes.  It describes how the body is translated in space.  It can be modified directly 
        using either :meth:`set_rotation` or :meth:`set_pose`.  Alternatively, it can be modified using 
        :meth:`spice_rotation` which uses predefined SPICE ephemerides to calculate a rotation for a 
        given ephemeris time.
        """

        self.name = name
        """
        NAIF name or ID code for the body (:code:`str`)
        
        This attribute can be used by the subclasses :class:`~.Entity`, as well as all :class:`~.cameras.Camera` and 
        :class:`~.lights.Light` classes.  It is used by the SPICE utility, :func:`spiceypy.spkpos`, as the name for 
        the celestial body whose position will be used as the position of the body.  This corresponds to 
        the :code:`targ` argument to :func:`spiceypy.spkpos`.  Please refer to the 
        `spkpos <https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/spkpos_c.html>`_ documentation 
        for more information.
        """

        self.frame = frame
        """
        NAIF reference frame name for the body (:code:`str`)
        
        This attribute can be used by the subclasses :class:`~.Entity`, as well as all :class:`~.cameras.Camera` and 
        :class:`~.lights.Light` classes.  It is used by the SPICE utility, :func:`spiceypy.pxform`, as the name for 
        the reference frame whose orientation will be used as the rotation of the body.  This corresponds to 
        the :code:`to` argument to :func:`spiceypy.pxform`.  Please refer to the 
        `pxform <https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/pxform_c.html>`_ documentation 
        for more information.
        """

        self.origin = origin
        """
        NAIF name or ID code for the reference frame origin (:code:`str`)
        
        This attribute can be used by the subclasses :class:`~.Entity`, as well as all :class:`~.cameras.Camera` and 
        :class:`~.lights.Light` classes.  It is used by the SPICE utility, :func:`spiceypy.spkpos`, as the name for 
        the celestial body whose position will be used as the origin of the reference frame.  This corresponds to 
        the :code:`obs` argument to :func:`spiceypy.spkpos`.  Please refer to the 
        `spkpos <https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/spkpos_c.html>`_ documentation 
        for more information.
        """
        
        self.ref = ref
        """
        NAIF reference frame name (:code:`str`)
        
        This attribute can be used by the subclasses :class:`~.Entity`, as well as all :class:`~.cameras.Camera` and 
        :class:`~.lights.Light` classes.  It is used by both the SPICE utilities, :func:`spiceypy.pxform` and 
        :func:`spiceypy.spkpos`, as the name for the reference frame against in which positions and
        rotations will be calculated.  This corresponds to the :code:`ref` argument to both :func:`spiceypy.spkpos`
        and :func:`spiceypy.pxform`.  Please refer to the 
        `spkpos <https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/spkpos_c.html>`_ or 
        `pxform <https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/pxform_c.html>`_ documentation for more 
        information.
        """

        self.abcorr = abcorr
        """
        Aberration correction flag (:code:`str`)

        This attribute is used by by the SPICE utility, :func:`spiceypy.spkpos`, as an indication of what
        light time correction should be used.  This corresponds to the :code:`abcorr` argument to 
        :func:`spiceypy.spkpos`.  Please refer to the 
        `spkpos <https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/spkpos_c.html>`_ documentation 
        for more information.
        """

    def set_scale(self, scale: float):
        """
        Set a new :attr:`~.RigidBody.scale`

        :param scale: Scale to be applied
        :type scale: float
        """
        self.scale = validate_scale(scale)

    def set_position(self, position: ArrayLike):
        """
        Set a new :attr:`~.RigidBody.position`

        :param position: Position to be applied (xyz)
        :type position: ArrayLike
        """
        self.position = validate_position(position)

    def set_rotation(self, rotation: ArrayLike):
        """
        Set a new :attr:`~.RigidBody.rotation`

        :param rotation: 3 by 3 Rotation matrix that must obey :math:`R^T = R^{-1}` and :math:`\det(R)=1`
        :type rotation: ArrayLike
        """
        self.rotation = validate_rotation(rotation)

    def set_pose(self, position: ArrayLike, rotation: ArrayLike):
        """
        Set both a new :attr:`~.RigidBody.position` and :attr:`~.RigidBody.rotation`

        :param position: Position to be applied (xyz)
        :type position: ArrayLike
        :param rotation: 3 by 3 Rotation matrix that must obey :math:`R^T = R^{-1}` and :math:`\det(R)=1`
        :type rotation: ArrayLike
        """
        self.position = validate_position(position)
        self.rotation = validate_rotation(rotation)

    def spice_position(self, et: float):
        """
        Set new position for a given ephemeris time using SPICE
        
        This is done using :func:`spiceypy.spkpos`, where the :attr:`~.RigidBody.name` is used as the :code:`targ` 
        argument, :attr:`~.RigidBody.ref` is used as the :code:`ref` argument, :attr:`~.RigidBody.abcorr` is 
        used as the :code:`abcorr` argument, and :attr:`~.RigidBody.origin` is used as the :code:`obs` argument.  
        Please refer to the 
        `spkpos <https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/spkpos_c.html>`_ documentation 
        for more information.

        :param et: Ephemeris time (seconds since the J2000 epoch), also known as Barycentric Dynamical Time
        :type et: float
        """
        position,_ = spkpos(self.name, et, self.ref, self.abcorr, self.origin)
        self.position = validate_position(position)

    def spice_rotation(self, et: float):
        """
        Set new rotation for a given ephemeris time using SPICE

        This is done using :func:`spiceypy.pxform`, where the :attr:`~.RigidBody.ref` is used as the :code:`from` 
        argument, and :attr:`~.RigidBody.frame` is used as the :code:`to` argumnet.  Please refer to the 
        `pxform <https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/pxform_c.html>`_ documentation 
        for more information.

        :param et: Ephemeris time (seconds since the J2000 epoch), also known as Barycentric Dynamical Time
        :type et: float
        """
        rotation = pxform(self.ref, self.frame, et)
        self.rotation = validate_rotation(rotation)

    def spice_pose(self, et: float):
        """
        Set both a new position and rotation for a given ephemeris time using SPICE

        The position is updated using :func:`spiceypy.spkpos`, where the :attr:`~.RigidBody.name` is used as 
        the :code:`targ` argument, :attr:`~.RigidBody.ref` is used as the :code:`ref` argument, 
        :attr:`~.RigidBody.abcorr` is used as the :code:`abcorr` argument, and :attr:`~.RigidBody.origin` is 
        used as the :code:`obs` argument.

        The rotation is updated using :func:`spiceypy.pxform`, where the :attr:`~.RigidBody.ref` is used as 
        the :code:`from` argument, and :attr:`~.RigidBody.frame` is used as the :code:`to` argumnet.
        
        Please refer to the 
        `spkpos <https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/spkpos_c.html>`_ or 
        `pxform <https://naif.jpl.nasa.gov/pub/naif/toolkit_docs/C/cspice/pxform_c.html>`_ documentation for more 
        information.

        :param et: Ephemeris time (seconds since the J2000 epoch), also known as Barycentric Dynamical Time
        :type et: float
        """
        position,_ = spkpos(self.name, et, self.ref, self.abcorr, self.origin)
        rotation   = pxform(self.ref, self.frame, et)
        self.position = validate_position(position)
        self.rotation = validate_rotation(rotation)