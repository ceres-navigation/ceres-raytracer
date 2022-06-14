import _crt
import numpy as np

def normal_pass(camera, entities, return_image=False):
    entities_cpp = []
    for entity in entities:
        entities_cpp.append(entity._cpp)

    normals = _crt.normal_pass(camera._cpp, entities_cpp)

    if return_image:
        image = 255*np.abs(normals)
        return normals, image

    return normals

def intersection_pass(camera, entities, return_image=False):
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

def instance_pass(camera, entities, return_image=False):
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