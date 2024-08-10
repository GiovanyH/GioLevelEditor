#pragma once
#include <glm/ext/matrix_float4x4.hpp>
#include <vector>
#include <array>
#include "Camera.h"

// Gizmo manipulation
void gizmo_transform_create(glm::mat4 camera_view, glm::mat4 camera_projection, std::vector<std::array<float, 16>> object_matrix, float* identity_matrix, int lastUsing, camera* editor_camera);
void gizmo_edit_transform(glm::mat4 camera_view, glm::mat4 camera_projection, float* matrix);
void gizmo_transform_end();