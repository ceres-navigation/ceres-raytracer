from build.src.ceres_rt import PinholeCamera, PointLight, Entity, CRT
import numpy as np
import cv2

# Create the camera object:
focal_length = 30
resolution = [1080, 1080]
sensor_size = [19.5, 19.5]
camera_position = np.array([10,0,0])
camera_rotation = np.array([[0, 1, 0],
                            [0, 0, -1],
                            [1, 0, 0]])
camera = PinholeCamera(focal_length, resolution, sensor_size)
camera.set_pose(camera_position, camera_rotation)

# Create the sun object:
intensity = 15
sun_position = np.array([3.0, 3.0, 10.0])
sun = PointLight(intensity)
sun.set_position(sun_position)

# Load the comet model:
model_path = "../test_data/67p.obj"
comet_position = np.array([0,0,0])
comet_rotation = np.array([[0.3420202, -0.9396926,  0.0000000],
                           [0.9396926,  0.3420202,  0.0000000],
                           [0.0000000,  0.0000000,  1.0000000]])
smooth_shading = True
color = [1,1,1]
comet_67p = Entity(model_path, smooth_shading, color)
comet_67p.set_pose(comet_position, comet_rotation)

# Create the CRT instance:
min_samples = 1
max_samples = 1
noise_threshold = 0.000001
num_bounces = 1
crt = CRT(min_samples, max_samples, noise_threshold, num_bounces)

# Perform rendering:
img = crt.render(camera, sun, comet_67p)
cv2.imwrite('demo.png', img)