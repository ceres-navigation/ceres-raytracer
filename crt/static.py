import _crt
import numpy as np

class StaticScene:
    def __init__(self, entities):
        self.entities = entities

        entities_cpp = []
        for entity in entities:
            entities_cpp.append(entity._cpp)

        self._cpp = _crt.StaticScene(entities_cpp)

    def render(self, camera, lights, min_samples=1, max_samples=1, noise_threshold=1, num_bounces=1):
        lights_cpp = []
        for light in lights:
            lights_cpp.append(light._cpp)

        image = self._cpp.render(camera._cpp, lights_cpp,
                                 min_samples, max_samples, noise_threshold, num_bounces)
        return image

class StaticEntity:
    def __init__(self, geometry_path, color=[1,1,1], smooth_shading=False, scale=1, position=np.zeros(3), rotation=np.eye(3)):
        self.geometry_path = geometry_path
        self.color = color
        self.smooth_shading = smooth_shading
        self.scale = scale
        self.position = position
        self.rotation = rotation

        self._cpp = _crt.StaticEntity(self.geometry_path, self.smooth_shading, self.color)
        self._cpp.set_pose(self.position, self.rotation)
        self._cpp.set_scale(self.scale)