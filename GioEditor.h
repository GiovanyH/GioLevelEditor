#pragma once
#include "OS/imgui_impl_glfw.h"
#include "OpenGL/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include "Camera/Camera.h"

class GioEditor {
private:
    int screen_height, screen_width;
    GLFWwindow* main_window = nullptr;
    unsigned int VAO = 0, VBO = 0;
    int firstFrame = false;
    int *lastUsing = new int(0);
    glm::mat4 camera_view;
    glm::mat4 camera_projection;
    camera* editor_camera;

    // Camera projection
    float fov = 27.f;
public:
    GioEditor();

    camera* get_cam();

    int init_glfw();

    int ready();
    int update();
    int clean_up();

    int window_should_close();
};