#pragma once

#include "Camera.h"

glm::vec3 get_direction_based_on_keys(bool forward, bool backward, bool right, bool left, bool up, bool down);
void fps_camera_control(camera* editor_camera, glm::vec3 direction, float window_width, float window_height);