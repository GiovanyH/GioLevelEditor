#pragma once
#include "../GameObject.h"
#include "../Camera/Camera.h"
#include <vector>
#include <array>

// Gizmo manipulation
void gizmo_transform_create(glm::mat4 camera_view, glm::mat4 camera_projection, std::vector<glm::mat4> object_matrix, float* identity_matrix, int lastUsing, camera* editor_camera, std::vector<mesh*> meshes);
void gizmo_edit_transform(glm::mat4 camera_view, glm::mat4 camera_projection, float* matrix);
void gizmo_transform_end();