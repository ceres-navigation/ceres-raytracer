import _crt
import numpy as np

class StaticScene:
    def __init__(self, entities, position=np.zeros(3), rotation=np.eye(3)):
        self.entities = entities

        entities_cpp = []
        for entity in entities:
            entities_cpp.append(entity._cpp)

        self._cpp = _crt.StaticScene(entities_cpp)

        position = position
        rotation = rotation

    def set_position(self, position):
        self.position = position
        self._cpp.set_position(position)

    def set_rotation(self, rotation):
        self.rotation = rotation
        self._cpp.set_rotation(rotation)

    def set_pose(self, position, rotation):
        self.position = position
        self.rotation = rotation
        self._cpp.set_pose(position, rotation)

    def render(self, camera, lights, min_samples=1, max_samples=1, noise_threshold=1, num_bounces=1):
        lights_cpp = []
        for light in lights:
            lights_cpp.append(light._cpp)

        image = self._cpp.render(camera._cpp, lights_cpp,
                                 min_samples, max_samples, noise_threshold, num_bounces)
        return image

    def normal_pass(self, camera, return_image = False):
        normals = self._cpp.normal_pass(camera._cpp)
        if return_image:
            image = 255*np.abs(normals)
            return normals, image
        return normals

    def intersection_pass(self, camera, return_image=False):
        intersections = self._cpp.intersection_pass(camera._cpp)
        if return_image:
            image = np.sqrt(intersections[:,:,0]**2 + intersections[:,:,1]**2 + intersections[:,:,2]**2)
            image = image - np.min(image)
            image = 255*image/np.max(image)
            return intersections, image
        return intersections

    def instance_pass(self, camera, return_image=False):
        instances = self._cpp.instance_pass(camera._cpp)
        if return_image:
            unique_ids = np.unique(instances)
            colors = np.random.randint(0, high=255, size=(3,unique_ids.size))
            image = np.zeros((instances.shape[0], instances.shape[1], 3))
            for idx, id in enumerate(unique_ids):
                mask = instances == id
                image[mask,:] = colors[:,idx]
            return instances, image
        return instances
            

class StaticEntity:
    def __init__(self, geometry_path, color=[1,1,1], geometry_type="obj", smooth_shading=False, scale=1, position=np.zeros(3), rotation=np.eye(3)):
        self.geometry_path = geometry_path
        self.geometry_type = geometry_type
        self.color = color
        self.smooth_shading = smooth_shading
        self.scale = scale
        self.position = position
        self.rotation = rotation

        self._cpp = _crt.StaticEntity(self.geometry_path, self.geometry_type, self.smooth_shading, self.color)
        self._cpp.set_pose(self.position, self.rotation)
        self._cpp.set_scale(self.scale)