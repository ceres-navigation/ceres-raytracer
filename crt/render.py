import _crt

def render(camera, lights, entities, min_samples=1, max_samples=1, noise_threshold=1, num_bounces=1):
    lights_cpp = []
    for light in lights:
        lights_cpp.append(light._cpp)
    
    entities_cpp = []
    for entity in entities:
        entities_cpp.append(entity._cpp)

    image = _crt.render(camera._cpp, lights_cpp, entities_cpp,
                        min_samples, max_samples, noise_threshold, num_bounces)
    return image