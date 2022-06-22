import _crt
import numpy as np

from crt._rigid_body import RigidBody
from crt._pybind_convert import validate_entities

class BodyFixedGroup(RigidBody):
    def __init__(self, entities, position=np.zeros(3), rotation=np.eye(3)):
        entities_cpp = validate_entities(entities)

        self._cpp = _crt.BodyFixedGroup(entities_cpp)
        self.set_pose(position, rotation, cpp=False)

    def render(self, camera, lights, min_samples=1, max_samples=1, noise_threshold=1, num_bounces=1):
        # Transform camera into BodyFixedGroupd frame:
        cam_rel_pos = camera.position - self.position
        cam_rel_pos = np.matmul(self.rotation, cam_rel_pos)
        cam_rel_rot = np.matmul(self.rotation, camera.rotation.T).T
        camera.set_pose(cam_rel_pos, cam_rel_rot)

        lights_cpp = []
        for light in lights:
            light_rel_pos = light.position - self.position
            light_rel_pos = np.matmul(self.rotation, light_rel_pos)
            light_rel_rot = np.matmul(self.rotation, light.rotation.T).T
            light.set_pose(light_rel_pos, light_rel_rot)
            lights_cpp.append(light._cpp)

        image = self._cpp.render(camera._cpp, lights_cpp,
                                 min_samples, max_samples, noise_threshold, num_bounces)
        return image

    def normal_pass(self, camera, return_image = False):
        # Transform camera into BodyFixedGroup frame:
        cam_rel_pos = camera.position - self.position
        cam_rel_pos = np.matmul(self.rotation, cam_rel_pos)
        cam_rel_rot = np.matmul(self.rotation, camera.rotation.T).T
        camera.set_pose(cam_rel_pos, cam_rel_rot)

        normals = self._cpp.normal_pass(camera._cpp)
        if return_image:
            image = 255*np.abs(normals)
            return normals, image
        return normals

    def intersection_pass(self, camera, return_image=False):
        # Transform camera into BodyFixedGroup frame:
        cam_rel_pos = camera.position - self.position
        cam_rel_pos = np.matmul(self.rotation, cam_rel_pos)
        cam_rel_rot = np.matmul(self.rotation, camera.rotation.T).T
        camera.set_pose(cam_rel_pos, cam_rel_rot)

        intersections = self._cpp.intersection_pass(camera._cpp)
        if return_image:
            image = np.sqrt(intersections[:,:,0]**2 + intersections[:,:,1]**2 + intersections[:,:,2]**2)
            image = image - np.min(image)
            image = 255*image/np.max(image)
            return intersections, image
        return intersections

    def instance_pass(self, camera, return_image=False):
        # Transform camera into BodyFixedGroup frame:
        cam_rel_pos = camera.position - self.position
        cam_rel_pos = np.matmul(self.rotation, cam_rel_pos)
        cam_rel_rot = np.matmul(self.rotation, camera.rotation.T).T
        camera.set_pose(cam_rel_pos, cam_rel_rot)

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
    def __init__(self, geometry_path, color=[1,1,1], geometry_type="obj", smooth_shading=False, scale=1, position=np.zeros(3), rotation=np.eye(3)):
        self.geometry_path = geometry_path
        self.geometry_type = geometry_type
        self.color = color
        self.smooth_shading = smooth_shading

        self._cpp = _crt.BodyFixedEntity(self.geometry_path, self.geometry_type, self.smooth_shading, self.color)
        self.set_pose(position, rotation)
        self.set_scale(scale)