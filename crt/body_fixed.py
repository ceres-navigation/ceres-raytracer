import _crt
import numpy as np
from typing import Union, List, Tuple
from numpy.typing import ArrayLike

from crt.cameras import Camera
from crt.lights import Light
from crt.lidars import Lidar

from crt.rigid_body import RigidBody

class BodyFixedEntity(RigidBody):
    """
    The :class:`BodyFixedEntity` class stores all mesh, material, and texture information for a body fixed geometry

    :param geometry_path: Path to the mesh geometry to be loaded
    :type geometry_path: str
    :param color: The RGB color code of the geometry |default| :code:`[1,1,1]`
    :type color: ArrayLike, optional
    :param geometry_type: The format of the mesh geoemtry provided |default| :code:`"obj"`
    :type geometry_type: str, optional
    :param smooth_shading: Flag to enable smooth shading via vertex normal interpolation |default| :code:`False`
    :type smooth_shading: bool, optional
    """
    def __init__(self, geometry_path: str, color: ArrayLike =[1,1,1], geometry_type: str="obj",
                 smooth_shading: bool=False, **kwargs):
        super(BodyFixedEntity, self).__init__(**kwargs)
        
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

        self._cpp = _crt.BodyFixedEntity(self.geometry_path, self.geometry_type, self.smooth_shading, self.color)
        """
        Corresponding C++ Entity object
        """
        
        self.set_pose(self.position, self.rotation)
        self.set_scale(self.scale)


class BodyFixedGroup(RigidBody):
    """
    Group of body fixed entities so that rendering occures in the body frame, allowing for the
    bounding volume heirarchy to be cached inbetween renderings

    :param entities: BodyFixedEntity/Entities against which ray tracing is performed
    :type entities: Union[BodyFixedEntity, List[BodyFixedEntity], Tuple[BodyFixedEntity,...]]
    """
    def __init__(self, entities: Union[BodyFixedEntity, List[BodyFixedEntity], Tuple[BodyFixedEntity,...]],
                 **kwargs):       
        super(BodyFixedGroup, self).__init__(**kwargs)

        err_msg = """error"""

        # Validate the input body fixed entities:
        entities_cpp = []
        if (type(entities) == list) or (type(entities) == tuple):
            for entity in entities:
                assert(type(entity) == BodyFixedEntity, err_msg)
                entities_cpp.append(entity._cpp)
        else:
            assert(type(entities) == BodyFixedEntity, err_msg)
            entities_cpp.append(entities._cpp)

        self._cpp = _crt.BodyFixedGroup(entities_cpp)
        """
        Corresponding C++ BodyFixedGroup object
        """

    def transform_to_body(self, position: ArrayLike, rotation: ArrayLike) -> Tuple[np.ndarray, np.ndarray]:
        """
        Transform provided position and rotation into the body fixed frame

        :param position: Input position represented in the base reference frame
        :type position: ArrayLike
        :param rotation: Input rotation represented in the base reference frame
        :type rotation: ArrayLike
        :return: position and rotation transformed into the body fixed frame
        :rtype: Tuple[np.ndarray, np.ndarray]
        """
        relative_position = position - self.position
        relative_position = self.scale*np.matmul(self.rotation, relative_position)
        relative_rotation = np.matmul(self.rotation, rotation.T).T
        return relative_position, relative_rotation

    # def transform_from_body(self, relative_position, relative_rotation):
    #     # TODO: Implement this
    #     return position, rotation

    def render(self, camera: Camera, lights: Union[Light, List[Light], Tuple[Light,...]],
              min_samples: int=1, max_samples: int=1, noise_threshold: float=1., num_bounces: int=1) -> np.ndarray:
        """
        Render a scene with a set of grouped body fixed entities.

        :param camera: Camera model to be used for generatring rays
        :type camera: Camera
        :param lights: ight(s) to be used for rendering
        :type lights: Union[Light, List[Light], Tuple[Light,...]]
        :param min_samples: Minimum number of ray samples per pixel |default| :code:`1`
        :type min_samples: int, optional
        :param max_samples: Maximum number of ray samples per pixel |default| :code:`1`
        :type max_samples: int, optional
        :param noise_threshold: Pixel noise threshold for adaptive sampling |default| :code:`1`
        :type noise_threshold: float, optional
        :param num_bounces: Number of ray bounces |default| :code:`1`
        :type num_bounces: int, optional
        :return: Rendered image
        :rtype: np.ndarray
        """
        # Transform camera into BodyFixedGroupd frame:
        relative_position, relative_rotation = self.transform_to_body(camera.position, camera.rotation)
        camera.set_pose(relative_position, relative_rotation)

        lights_cpp = []
        if (type(lights) is list) or (type(lights) is tuple):
            for light in lights:
                relative_position, relative_rotation = self.transform_to_body(light.position, light.rotation)
                light.set_pose(relative_position, relative_rotation)
                lights_cpp.append(light._cpp)
        else:
            relative_position, relative_rotation = self.transform_to_body(lights.position, lights.rotation)
            lights.set_pose(relative_position, relative_rotation)
            lights_cpp.append(lights._cpp)

        image = self._cpp.render(camera._cpp, lights_cpp,
                                 min_samples, max_samples, noise_threshold, num_bounces)
        return image

    def simulate_lidar(self, lidar: Lidar, num_rays: int=1):
        relative_position, relative_rotation = self.transform_to_body(lidar.position, lidar.rotation)
        lidar.set_pose(relative_position, relative_rotation)
        distance = self._cpp.simulate_lidar(lidar._cpp, num_rays)
        return distance

    def batch_simulate_lidar(self, lidar: Lidar, num_rays: int=1):
        positions = lidar.batch_positions
        rotations = lidar.batch_rotations
        relative_positions = np.zeros(positions.shape)
        relative_rotations = np.zeros(rotations.shape)
        for idx in range(0,positions.shape[0]):
            relative_positions[idx,:], relative_rotations[:,:,idx] = self.transform_to_body(positions[idx,:], rotations[:,:,idx])

        lidar.batch_set_pose(relative_positions, relative_rotations)

        distances = self._cpp.batch_simulate_lidar(lidar._cpp, num_rays)
        return distances

    def normal_pass(self, camera: Camera, 
                    return_image: bool = False) -> Union[np.ndarray, Tuple[np.ndarray, np.ndarray]]:
        """
        Perform a normal pass with body fixed entities

        :param camera: Camera model to be used for generating rays
        :type camera: Camera
        :param return_image: Flag to return an image representation of the intersected normals |default| :code:`False`
        :type return_image: bool, optional
        :return: An array of the intersected normals.  If :code:`return_image` is set to :code:`True`, then an image
                where the normal XYZ values are represented using RGB color values is returned as a second output.
        :rtype: Union[np.ndarray, Tuple(np.ndarray, np.ndarray)]
        """
        # Transform camera into BodyFixedGroup frame:
        relative_position, relative_rotation = self.transform_to_body(camera.position, camera.rotation)
        camera.set_pose(relative_position, relative_rotation)

        normals = self._cpp.normal_pass(camera._cpp)
        if return_image:
            image = 255*np.abs(normals)
            return normals, image
        return normals

    def intersection_pass(self, camera: Camera,
                          return_image: bool=False) -> Union[np.ndarray, Tuple[np.ndarray, np.ndarray]]:
        """
        Perform a normal pass with body fixed entities

        :param camera: Camera model to be used for generating rays
        :type camera: Camera
        :param return_image: Flag to return an image representation of the intersection depth |default| :code:`False`
        :type return_image: bool, optional
        :return: An array of the intersected points.  If :code:`return_image` is set to :code:`True`, then an image
                where the distance to each intersected point is represented via pixel intensity is returned 
                as a second output.
        :rtype: Union[np.ndarray, Tuple(np.ndarray, np.ndarray)]
        """
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

    def instance_pass(self, camera: Camera, 
                      return_image: bool=False) -> Union[np.ndarray, Tuple[np.ndarray, np.ndarray]]:
        """
        Perform an instance segmentation pass with body fixed entities

        :param camera: Camera model to be used for generating rays
        :type camera: Camera
        :param return_image: Flag to return an image representation of the instances |default| :code:`False`
        :type return_image: bool, optional
        :return: An array unique id codes for each unique entity intersected.  If :code:`return_image` is set 
                to :code:`True`, then an image where each unique id is represented with a unique RGB color 
                is returned as a second output.
        :rtype: Union[np.ndarray, Tuple(np.ndarray, np.ndarray)]
        """
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