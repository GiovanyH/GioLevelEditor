#pragma once
#include "OpenGL/imgui_impl_opengl3.h"
#include "OS/imgui_impl_glfw.h"
#include <GLFW/glfw3.h>
#include <string>
#include <imgui.h>
#include <imgui_internal.h>
#include "ImGuizmo.h"

#include <math.h>
#include <vector>
#include <algorithm>
#include <iostream>

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>

class GioEditor {
private:
    int screen_height, screen_width;
    GLFWwindow* main_window = nullptr;
    unsigned int VAO = 0, VBO = 0;
    int firstFrame = false;

    int lastUsing = 0;

    glm::mat4 camera_view;

    glm::mat4 camera_projection;

    // Camera projection
    float fov = 27.f;
public:
    GioEditor();

    int init_glfw();

    int ready();
    int update();
    int clean_up();

    int window_should_close();

    int is_first_frame();
};