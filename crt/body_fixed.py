import _crt
import numpy as np

from crt.rigid_body import RigidBody
from crt._pybind_convert import validate_entities

class BodyFixedGroup(RigidBody):
    def __init__(self, entities, **kwargs):
        super(BodyFixedGroup, self).__init__(**kwargs)

        entities_cpp = validate_entities(entities)

        # Create corresponding C++ object:
        self._cpp = _crt.BodyFixedGroup(entities_cpp)
        self.set_pose(self.position, self.rotation, cpp=False)
        self.set_scale(self.scale, cpp=False)

    def transform_to_body(self, position, rotation):
        relative_position = position - self.position
        relative_position = np.matmul(self.rotation, relative_position)
        relative_rotation = np.matmul(self.rotation, rotation.T).T
        return relative_position, relative_rotation

    # def transform_from_body(self, relative_position, relative_rotation):
    #     # TODO: Implement this
    #     return position, rotation

    def render(self, camera, lights, min_samples=1, max_samples=1, noise_threshold=1, num_bounces=1):
        # Transform camera into BodyFixedGroupd frame:
        relative_position, relative_rotation = self.transform_to_body(camera.position, camera.rotation)
        camera.set_pose(relative_position, relative_rotation)

        lights_cpp = []
        for light in lights:
            relative_position, relative_rotation = self.transform_to_body(light.position, light.rotation)
            light.set_pose(relative_position, relative_rotation)
            lights_cpp.append(light._cpp)

        image = self._cpp.render(camera._cpp, lights_cpp,
                                 min_samples, max_samples, noise_threshold, num_bounces)
        return image

    def normal_pass(self, camera, return_image = False):
        # Transform camera into BodyFixedGroup frame:
        relative_position, relative_rotation = self.transform_to_body(camera.position, camera.rotation)
        camera.set_pose(relative_position, relative_rotation)

        normals = self._cpp.normal_pass(camera._cpp)
        if return_image:
            image = 255*np.abs(normals)
            return normals, image
        return normals

    def intersection_pass(self, camera, return_image=False):
        # Transform camera into BodyFixedGroup frame:
        relative_position, relative_rotation = self.transform_to_body(camera.position, camera.rotation)
        camera.set_pose(relative_position, relative_rotation)

        intersections = self._cpp.intersection_pass(camera._cpp)
        if return_image:
            image = np.sqrt(intersections[:,:,0]**2 + intersections[:,:,1]**2 + intersections[:,:,2]**2)
            image = image - np.min(image)
            image = 255*image/np.max(image)
            return intersections, image
        return intersections

    def instance_pass(self, camera, return_image=False):
        # Transform camera into BodyFixedGroup frame:
        relative_position, relative_rotation = self.transform_to_body(camera.position, camera.rotation)
        camera.set_pose(relative_position, relative_rotation)

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
            

class BodyFixedEntity(RigidBody):
    def __init__(self, geometry_path, color=[1,1,1], geometry_type="obj", smooth_shading=False, **kwargs):
        self.geometry_path = geometry_path
        self.geometry_type = geometry_type
        self.color = color
        self.smooth_shading = smooth_shading

        # Create corresponding C++ object:
        self._cpp = _crt.BodyFixedEntity(self.geometry_path, self.geometry_type, self.smooth_shading, self.color)
        self.set_pose(self.position, self.rotation)
        self.set_scale(self.scale)