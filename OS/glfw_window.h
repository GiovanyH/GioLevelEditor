
// TODO: Move this to somewhere else
/* GLFW */
void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

int GioEditor::init_glfw() {
    // setup glfw
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return EXIT_FAILURE;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create main window
    this->main_window = glfwCreateWindow(this->screen_width, this->screen_height, "Demo: OpenGL Only", nullptr, nullptr);

    glfwMakeContextCurrent(this->main_window);

    return EXIT_SUCCESS;
}
int GioEditor::window_should_close() {
    return glfwWindowShouldClose(this->main_window);
}
/* == */