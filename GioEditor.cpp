#include "GioEditor.h"
#include "Camera.h"

camera first_camera;

/* constructors */
GioEditor::GioEditor() {
    this->screen_width = 1280;
    this->screen_height = 720;
}

/* all static function definitions */
void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

void style_color_softy(ImGuiStyle* dst = nullptr)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();

    int hspacing = 8;
    int vspacing = 6;
    style->DisplaySafeAreaPadding = ImVec2(0, 0);
    style->WindowPadding = ImVec2(hspacing / 2, vspacing);
    style->FramePadding = ImVec2(hspacing, vspacing);
    style->ItemSpacing = ImVec2(hspacing, vspacing);
    style->ItemInnerSpacing = ImVec2(hspacing, vspacing);
    style->IndentSpacing = 20.0f;

    style->WindowRounding = 0.0f;
    style->FrameRounding = 0.0f;

    style->WindowBorderSize = 0.0f;
    style->FrameBorderSize = 1.0f;
    style->PopupBorderSize = 1.0f;

    style->ScrollbarSize = 20.0f;
    style->ScrollbarRounding = 0.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 0.0f;

    ImVec4 white = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    ImVec4 transparent = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    ImVec4 dark = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
    ImVec4 darker = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);

    ImVec4 background = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    ImVec4 text = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    ImVec4 border = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    ImVec4 grab = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    ImVec4 header = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    ImVec4 active = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);
    ImVec4 hover = ImVec4(0.00f, 0.47f, 0.84f, 0.20f);

    style->Colors[ImGuiCol_Text] = text;
    style->Colors[ImGuiCol_WindowBg] = background;
    style->Colors[ImGuiCol_ChildBg] = background;
    style->Colors[ImGuiCol_PopupBg] = white;

    style->Colors[ImGuiCol_Border] = border;
    style->Colors[ImGuiCol_BorderShadow] = transparent;

    style->Colors[ImGuiCol_Button] = header;
    style->Colors[ImGuiCol_ButtonHovered] = hover;
    style->Colors[ImGuiCol_ButtonActive] = active;

    style->Colors[ImGuiCol_FrameBg] = white;
    style->Colors[ImGuiCol_FrameBgHovered] = hover;
    style->Colors[ImGuiCol_FrameBgActive] = active;

    style->Colors[ImGuiCol_MenuBarBg] = header;
    style->Colors[ImGuiCol_Header] = header;
    style->Colors[ImGuiCol_HeaderHovered] = hover;
    style->Colors[ImGuiCol_HeaderActive] = active;

    style->Colors[ImGuiCol_CheckMark] = text;
    style->Colors[ImGuiCol_SliderGrab] = grab;
    style->Colors[ImGuiCol_SliderGrabActive] = darker;

    style->Colors[ImGuiCol_ScrollbarBg] = header;
    style->Colors[ImGuiCol_ScrollbarGrab] = grab;
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = dark;
    style->Colors[ImGuiCol_ScrollbarGrabActive] = darker;
}

/* non-static */
int GioEditor::init_glfw() 
{
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

bool use_window = true;
int gizmo_count = 1;
float cam_distance = 8.f;

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);

float object_matrix[4][16] = {
  { 1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f },

  { 1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  2.f, 0.f, 0.f, 1.f },

  { 1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  2.f, 0.f, 2.f, 1.f },

  { 1.f, 0.f, 0.f, 0.f,
  0.f, 1.f, 0.f, 0.f,
  0.f, 0.f, 1.f, 0.f,
  0.f, 0.f, 2.f, 1.f }
};

static const float identity_matrix[16] =
{ 1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f };

glm::vec3 old_mouse_pos;

void fps_camera_mode()
{
    glm::vec3 forward = get_forward_vector(first_camera.yaw, first_camera.pitch);
    float velocity = 0.01;

    if (ImGui::IsKeyDown(ImGuiKey_W)) {
        first_camera.position += forward * velocity;
    }
    if (ImGui::IsKeyDown(ImGuiKey_S)) {
        first_camera.position -= forward * velocity;
    }
    if (ImGui::IsKeyDown(ImGuiKey_D)) {
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        first_camera.position += right * velocity;
    }
    if (ImGui::IsKeyDown(ImGuiKey_A)) {
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        first_camera.position -= right * velocity;
    }
    if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
        first_camera.position.y += 0.01;
    }
    if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
        first_camera.position.y -= 0.01;
    }
}

void idle_camera_mode()
{
    if (ImGui::IsKeyPressed(ImGuiKey_W))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E)) // r Key
        mCurrentGizmoOperation = ImGuizmo::SCALE;
}

void gizmo_transform_create(glm::mat4 camera_view, glm::mat4 camera_projection, float* matrix)
{
    static float old_pitch;
    static float old_yaw;
    static bool camera_mode = false;

    ImGuiIO& io = ImGui::GetIO();

    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        first_camera.pitch = old_pitch + (old_mouse_pos.x - io.MousePos.x) / 180.0f;
        first_camera.yaw = old_yaw + (old_mouse_pos.y - io.MousePos.y) / 180.0f;

        // Constrain the pitch to prevent screen flipping
        if (first_camera.pitch > 89.0f)
            first_camera.pitch = 89.0f;
        if (first_camera.pitch < -89.0f)
            first_camera.pitch = -89.0f;

        camera_mode = true;
    }
    else {
        old_mouse_pos.x = io.MousePos.x;
        old_mouse_pos.y = io.MousePos.y;

        old_pitch = first_camera.pitch;
        old_yaw = first_camera.yaw;

        camera_mode = false;
    }

    if (camera_mode) fps_camera_mode();
    else idle_camera_mode();

    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
        mCurrentGizmoOperation = ImGuizmo::SCALE;

    float matrix_translation[3], matrix_rotation[3], matrix_scale[3];
    ImGuizmo::DecomposeMatrixToComponents(matrix, matrix_translation, matrix_rotation, matrix_scale);
    ImGui::InputFloat3("Tr", matrix_translation);
    ImGui::InputFloat3("Rt", matrix_rotation);
    ImGui::InputFloat3("Sc", matrix_scale);
    ImGuizmo::RecomposeMatrixFromComponents(matrix_translation, matrix_rotation, matrix_scale, matrix);

    if (mCurrentGizmoOperation != ImGuizmo::SCALE)
    {
        if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
            mCurrentGizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
            mCurrentGizmoMode = ImGuizmo::WORLD;
    }

    float view_manipulate_right = io.DisplaySize.x;
    float view_manipulate_top = 0;
    static ImGuiWindowFlags gizmo_window_flags = 0;
    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(330, 10), ImGuiCond_Appearing);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
    ImGui::Begin("Gizmo", 0, gizmo_window_flags);
    ImGuizmo::SetDrawlist();
    float window_width = (float)ImGui::GetWindowWidth();
    float window_height = (float)ImGui::GetWindowHeight();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, window_width, window_height);
    view_manipulate_right = ImGui::GetWindowPos().x + window_width;
    view_manipulate_top = ImGui::GetWindowPos().y;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    gizmo_window_flags = ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(window->InnerRect.Min, window->InnerRect.Max) ? ImGuiWindowFlags_NoMove : 0;

    ImGuizmo::DrawGrid((const float*)glm::value_ptr(camera_view), (const float*)glm::value_ptr(camera_projection), identity_matrix, 100.f);
    ImGuizmo::DrawCubes((const float*)glm::value_ptr(camera_view), (const float*)glm::value_ptr(camera_projection), &object_matrix[0][0], gizmo_count);

    ImGuizmo::ViewManipulate((float*)glm::value_ptr(camera_view), cam_distance, ImVec2(view_manipulate_right - 128, view_manipulate_top), ImVec2(128, 128), 0x10101010);
}

void gizmo_transform_end()
{
    ImGui::End();
    ImGui::PopStyleColor(1);
}

void gizmo_edit_transform(glm::mat4 camera_view, glm::mat4 camera_projection, float* matrix)
{
    ImGuiIO& io = ImGui::GetIO();
    float windowWidth = (float)ImGui::GetWindowWidth();
    float windowHeight = (float)ImGui::GetWindowHeight();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
    ImGuizmo::Manipulate((const float*)glm::value_ptr(camera_view), (const float*)glm::value_ptr(camera_projection), mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, NULL);
}

int GioEditor::ready()
{
    // setup glfw
    this->init_glfw();

    glfwMakeContextCurrent(this->main_window);
    glfwSwapInterval(0); // Enable vsync

    // load GL binders (either GLAD, GL3W or OpenGL)
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(this->main_window, true);
    ImGui_ImplOpenGL3_Init();

    // Setup style
    ImGui::StyleColorsDark();

    auto font_default = io.Fonts->AddFontDefault();

    unsigned int procTexture;
    glGenTextures(1, &procTexture);
    glBindTexture(GL_TEXTURE_2D, procTexture);
    uint32_t* tempBitmap = new uint32_t[256 * 256];
    int index = 0;
    for (int y = 0; y < 256; y++)
    {
        for (int x = 0; x < 256; x++)
        {
            float dx = x + .5f;
            float dy = y + .5f;
            float dv = sinf(x * 0.02f) + sinf(0.03f * (x + y)) + sinf(sqrtf(0.4f * (dx * dx + dy * dy) + 1.f));

            tempBitmap[index] = 0xFF000000 +
                (int(255 * fabsf(sinf(dv * 3.141592f))) << 16) +
                (int(255 * fabsf(sinf(dv * 3.141592f + 2 * 3.141592f / 3))) << 8) +
                (int(255 * fabs(sin(dv * 3.141592f + 4.f * 3.141592f / 3.f))));

            index++;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempBitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    delete[] tempBitmap;

    firstFrame = true;

    style_color_softy(&ImGui::GetStyle());

    return 0;
}

int GioEditor::update() {
    // Main loop
    glfwPollEvents();

    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();

    //Perspective(fov, io.DisplaySize.x / io.DisplaySize.y, 0.1f, 100.f, cameraProjection);
    camera_projection = glm::perspective(fov, io.DisplaySize.x / io.DisplaySize.y, 0.1f, 100.0f);
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    // create a window and insert the inspector
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(320, 400));
    ImGui::Begin("Editor");

    ImGui::Text("Camera");
    bool viewDirty = false;

    ImGui::SliderFloat("Fov", &fov, 20.f, 110.f);

    viewDirty |= ImGui::SliderFloat("Distance", &cam_distance, 1.f, 10.f);
    ImGui::SliderInt("Gizmo count", &gizmo_count, 1, 4);

    camera_view = glm::lookAt(first_camera.position, get_look_at(first_camera.position, first_camera.yaw, first_camera.pitch), glm::vec3(0, 1, 0));
    firstFrame = false;

    ImGui::Text("X: %f Y: %f", io.MousePos.x, io.MousePos.y);
    if (ImGuizmo::IsUsing())
    {
        ImGui::Text("Using gizmo");
    }
    else
    {
        ImGui::Text(ImGuizmo::IsOver() ? "Over gizmo" : "");
        ImGui::SameLine();
        ImGui::Text(ImGuizmo::IsOver(ImGuizmo::TRANSLATE) ? "Over translate gizmo" : "");
        ImGui::SameLine();
        ImGui::Text(ImGuizmo::IsOver(ImGuizmo::ROTATE) ? "Over rotate gizmo" : "");
        ImGui::SameLine();
        ImGui::Text(ImGuizmo::IsOver(ImGuizmo::SCALE) ? "Over scale gizmo" : "");
    }
    ImGui::Separator();

    gizmo_transform_create(camera_view, camera_projection, object_matrix[lastUsing]);
    
    for (int matId = 0; matId < gizmo_count; matId++)
    {
        ImGuizmo::SetID(matId);

        gizmo_edit_transform(camera_view, camera_projection, object_matrix[matId]);
        if (ImGuizmo::IsUsing())
        {
            lastUsing = matId;
        }
    }
    gizmo_transform_end();

    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(10, 350));

    ImGui::SetNextWindowSize(ImVec2(940, 480));

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(main_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    // rendering
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // everything falls under glUserProgram will impacted by the shader programs
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // before finishing current frame
    glfwSwapBuffers(main_window);
    glfwPollEvents();

    return 0;
}

int GioEditor::clean_up()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(this->main_window);
    glfwTerminate();

    return 0;
}