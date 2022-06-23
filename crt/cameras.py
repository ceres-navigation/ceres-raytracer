import numpy as np

import _crt
from crt.rigid_body import RigidBody

class PinholeCamera(RigidBody):
    def __init__(self, focal_length, resolution, sensor_size, z_positive=False, **kwargs):
        super(PinholeCamera, self).__init__(**kwargs)

        self.focal_length = focal_length
        self.resolution = resolution
        self.sensor_size = sensor_size

        # Create corresponding C++ object:
        self._cpp = _crt.PinholeCamera(focal_length, resolution, sensor_size,z_positive)
        self.set_pose(self.position, self.rotation)

    def get_fov(self, degrees=True):
        if degrees:
            return np.rad2deg(2*np.arctan2(self.sensor_size/2, 2*self.focal_length))
        else:
            return 2*np.arctan2(self.sensor_size/2, 2*self.focal_length)
    
        