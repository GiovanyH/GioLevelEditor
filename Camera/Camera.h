#pragma once
#include <glm/glm.hpp>

class camera
{
public:
	float yaw;
	float pitch;

	glm::vec3 position;

	glm::vec3 get_screen_space_position(glm::vec3 offset);
};

glm::vec3 get_look_at(glm::vec3 position, float yaw, float pitch);
glm::vec3 get_forward_vector(float yaw, float pitch);