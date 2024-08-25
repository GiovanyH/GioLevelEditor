#include "Camera.h"

glm::vec3 get_look_at(glm::vec3 position, float yaw, float pitch) {
    return position + glm::vec3(cos(pitch) * sin(yaw), sin(pitch), cos(pitch) * cos(yaw));
}

glm::vec3 get_forward_vector(float yaw, float pitch) {
    glm::vec3 forward;
    forward.x = cos(pitch) * sin(yaw);
    forward.y = sin(pitch);
    forward.z = cos(pitch) * cos(yaw);

    return glm::normalize(forward);
}

glm::vec3 camera::get_screen_space_position(glm::vec3 offset) {
    return this->position + offset;
}