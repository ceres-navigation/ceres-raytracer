from crt.lights import PointLight, AreaLight
from crt import Entity

def valid_light(light):
    return (type(light) == PointLight) or \
           (type(light) == AreaLight)

def validate_lights(lights):
    err_msg = """error"""

    lights_cpp = []
    if (type(lights) is list) or (type(lights) is tuple):
        for light in lights:
            assert(valid_light(light), err_msg)
            lights_cpp.append(light._cpp)
    else:
        assert(valid_light(lights), err_msg)
        lights_cpp.append(lights._cpp)

    return lights_cpp

def validate_entities(entities):
    err_msg = """error"""

    entities_cpp = []
    if (type(entities) == list) or (type(entities) == tuple):
        for entity in entities:
            assert(type(entity) == Entity, err_msg)
            entities_cpp.append(entity._cpp)
    else:
        assert(type(entities) == Entity, err_msg)
        entities_cpp.append(entities._cpp)

    return entities_cpp