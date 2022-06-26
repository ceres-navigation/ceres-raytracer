import _crt
import numpy as np

from typing import Union, List, Tuple
from crt import Entity
from crt.cameras import Camera
from crt.lights import Light
from crt.lidars import Lidar

from crt._pybind_convert import validate_lights, validate_entities

def render(camera: Camera, lights: Union[Light, List[Light], Tuple[Light,...]],
           entities: Union[Entity, List[Entity], Tuple[Entity,...]], 
           min_samples: int=1, max_samples: int=1, noise_threshold: float=1., num_bounces: int=1) -> np.ndarray:
    """
    Render a scene with dynamic entities.  Prior to rendering, a Bounding Volume Heirarchy will be built
    from scratch for the entire scene

    :param camera: Camera model to be used for generatring rays
    :type camera: Camera
    :param lights: Light(s) to be used for rendering
    :type lights: Union[Light, List[Light], Tuple[Light,...]]
    :param entities: Entity/Entities to be rendered
    :type entities: Union[Entity, List[Entity], Tuple[Entity,...]]
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
    lights_cpp = validate_lights(lights)

    entities_cpp = validate_entities(entities)

    image = _crt.render(camera._cpp, lights_cpp, entities_cpp,
                        min_samples, max_samples, noise_threshold, num_bounces)
    return image

def simulate_lidar(lidar: Lidar, entities: Union[Entity, List[Entity], Tuple[Entity,...]],
                   num_rays: int=1):

    entities_cpp = validate_entities(entities)

    distance = _crt.simulate_lidar(lidar._cpp, entities_cpp, num_rays)

    return distance


def normal_pass(camera: Camera, entities: Union[Entity, List[Entity], Tuple[Entity,...]],
                return_image: bool=False) -> Union[np.ndarray, Tuple[np.ndarray, np.ndarray]]:
                
    """
    Perform a normal pass with dynamic entities

    :param camera: Camera model to be used for generating rays
    :type camera: Camera
    :param entities: Entity/Entities against which ray tracing is performed
    :type entities: Union[Entity, List[Entity], Tuple[Entity,...]]
    :param return_image: Flag to return an image representation of the intersected normals |default| :code:`False`
    :type return_image: bool, optional
    :return: An array of the intersected normals.  If :code:`return_image` is set to :code:`True`, then an image
             where the normal XYZ values are represented using RGB color values is returned as a second output.
    :rtype: Union[np.ndarray, Tuple(np.ndarray, np.ndarray)]
    """
    entities_cpp = []
    for entity in entities:
        entities_cpp.append(entity._cpp)

    normals = _crt.normal_pass(camera._cpp, entities_cpp)

    if return_image:
        image = 255*np.abs(normals)
        return normals, image

    return normals

def intersection_pass(camera: Camera, entities: Union[Entity, List[Entity], Tuple[Entity,...]],
                      return_image: bool=False) -> Union[np.ndarray, Tuple[np.ndarray, np.ndarray]]:
    """
    Perform a normal pass with dynamic entities

    :param camera: Camera model to be used for generating rays
    :type camera: Camera
    :param entities: Entity/Entities against which ray tracing is performed
    :type entities: Union[Entity, List[Entity], Tuple[Entity,...]]
    :param return_image: Flag to return an image representation of the intersection depth |default| :code:`False`
    :type return_image: bool, optional
    :return: An array of the intersected points.  If :code:`return_image` is set to :code:`True`, then an image
             where the distance to each intersected point is represented via pixel intensity is returned 
             as a second output.
    :rtype: Union[np.ndarray, Tuple(np.ndarray, np.ndarray)]
    """
    entities_cpp = []
    for entity in entities:
        entities_cpp.append(entity._cpp)

    intersections = _crt.intersection_pass(camera._cpp, entities_cpp)

    if return_image:
        image = np.sqrt(intersections[:,:,0]**2 + intersections[:,:,1]**2 + intersections[:,:,2]**2)
        image = image - np.min(image)
        image = 255*image/np.max(image)
        return intersections, image

    return intersections

def instance_pass(camera: Camera, entities: Union[Entity, List[Entity], Tuple[Entity,...]], 
                  return_image: bool=False) -> Union[np.ndarray, Tuple[np.ndarray, np.ndarray]]:
    """
    Perform an instance segmentation pass with dynamic entities

    :param camera: Camera model to be used for generating rays
    :type camera: Camera
    :param entities: Entity/Entities against which ray tracing is performed
    :type entities: Union[Entity, List[Entity], Tuple[Entity,...]]
    :param return_image: Flag to return an image representation of the instances |default| :code:`False`
    :type return_image: bool, optional
    :return: An array unique id codes for each unique entity intersected.  If :code:`return_image` is set 
             to :code:`True`, then an image where each unique id is represented with a unique RGB color 
             is returned as a second output.
    :rtype: Union[np.ndarray, Tuple(np.ndarray, np.ndarray)]
    """

    entities_cpp = []
    for entity in entities:
        entities_cpp.append(entity._cpp)

    instances = _crt.instance_pass(camera._cpp, entities_cpp)
    
    if return_image:
        unique_ids = np.unique(instances)
        colors = np.random.randint(0, high=255, size=(3,unique_ids.size))
        image = np.zeros((instances.shape[0], instances.shape[1], 3))
        for idx, id in enumerate(unique_ids):
            mask = instances == id
            image[mask,:] = colors[:,idx]
        return instances, image

    return instances