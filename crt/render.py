import _crt
from crt._pybind_convert import validate_lights, validate_entities

def render(camera, lights, entities, min_samples=1, max_samples=1, noise_threshold=1, num_bounces=1):
    lights_cpp = validate_lights(lights)

    entities_cpp = validate_entities(entities)

    image = _crt.render(camera._cpp, lights_cpp, entities_cpp,
                        min_samples, max_samples, noise_threshold, num_bounces)
    return image