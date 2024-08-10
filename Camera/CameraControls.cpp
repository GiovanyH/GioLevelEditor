#include "CameraControls.h"

glm::vec3 get_direction_based_on_keys(bool forward, bool backward, bool right, bool left, bool up, bool down)
{
    glm::vec3 direction(0.0f, 0.0f, 0.0f);

    if (forward) {
        direction.z += 1.0f;
    }
    if (backward) {
        direction.z -= 1.0f;
    }
    if (right) {
        direction.x += 1.0f;
    }
    if (left) {
        direction.x -= 1.0f;
    }
    if (up) {
        direction.y += 1.0f;
    }
    if (down) {
        direction.y -= 1.0f;
    }

    return direction;
}

#include <GLFW/glfw3.h> // Include the GLFW header for cursor functions

void fps_camera_control(camera* editor_camera, glm::vec3 direction, float window_width, float window_height)
{
    // Get the GLFW window
    GLFWwindow* window = glfwGetCurrentContext();

    // Calculate the center of the window
    glm::vec2 screen_center(window_width / 2.0f, window_height / 2.0f);

    // Get the current mouse position
    double mouse_x, mouse_y;
    glfwGetCursorPos(window, &mouse_x, &mouse_y);

    // Calculate the delta from the center
    glm::vec2 mouse_delta = glm::vec2(mouse_x - screen_center.x, screen_center.y - mouse_y);

    // Update the yaw and pitch based on the mouse delta
    editor_camera->yaw -= mouse_delta.x / 180.0f;
    editor_camera->pitch += mouse_delta.y / 180.0f;

    // Constrain the pitch to prevent screen flipping
    if (editor_camera->pitch > 89.0f)
        editor_camera->pitch = 89.0f;
    if (editor_camera->pitch < -89.0f)
        editor_camera->pitch = -89.0f;

    // Reset the mouse position to the center
    glfwSetCursorPos(window, screen_center.x, screen_center.y);

    // Move the camera based on the direction vector
    glm::vec3 forward = get_forward_vector(editor_camera->yaw, editor_camera->pitch);
    float velocity = 0.01f;

    // Move forward or backward based on the z-component of direction
    editor_camera->position += forward * direction.z * velocity;

    // Move right or left based on the x-component of direction
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    editor_camera->position += right * direction.x * velocity;

    // Move up or down based on the y-component of direction
    editor_camera->position.y += direction.y * velocity;
}
