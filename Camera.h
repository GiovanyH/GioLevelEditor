#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef struct
{
	float yaw;
	float pitch;

	glm::vec3 position;
} camera;

glm::vec3 get_look_at(glm::vec3 position, float pitch, float yaw) {
	return position + glm::vec3(cos(pitch) * sin(yaw), sin(pitch), cos(pitch) * cos(yaw));
}

glm::vec3 get_forward_vector(float pitch, float yaw) {
    glm::vec3 forward;
    forward.x = cos(pitch) * sin(yaw);
    forward.y = sin(pitch);
    forward.z = cos(pitch) * cos(yaw);

    return glm::normalize(forward);
}