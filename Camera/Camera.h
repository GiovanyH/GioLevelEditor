#pragma once
#include <glm/glm.hpp>

typedef struct
{
	float yaw;
	float pitch;

	glm::vec3 position;
} camera;

glm::vec3 get_look_at(glm::vec3 position, float yaw, float pitch);
glm::vec3 get_forward_vector(float yaw, float pitch);