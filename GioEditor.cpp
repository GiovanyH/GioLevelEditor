#include "GioEditor.h"
#include "Camera.h"
#include "CameraControls.h"
#include "GizmoGUI.h"
#include "Style.h"

/* constructors */
// these are going to be here for now
GioEditor::GioEditor() {
    this->screen_width = 1280;
    this->screen_height = 720;
}

camera* GioEditor::get_cam() {
    return editor_camera;
}

// glfw initialization
void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

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

// objects matrices
std::vector<std::array<float, 16>> object_matrix;

static const float identity_matrix[16] =
{ 1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f };

// Camera controls
glm::vec3 old_mouse_pos;

// scene creation
root* root_scene;

// GUI stuff
void object_instance_window()
{
    ImGui::SetNextWindowPos(ImVec2(1135, 10));
    ImGui::SetNextWindowSize(ImVec2(140, 400));
    ImGui::Begin("object_instance_window", 0);
    static std::vector<char *> items;
    static int item_current = 0;
    ImGui::ListBox(" ", &item_current, items.data(), items.size(), 10);
    ImGui::SameLine();
    if (ImGui::Button("+", ImVec2(20, 20))) {
        node* current_scene = get_node_from_id(root_scene->current_scene);
        mesh* new_object = (mesh*)create_new_object("mesh_object", "mesh");
        current_scene->add_child(new_object);
        object_matrix.push_back(new_object->transform);
        items.push_back((char*)new_object->name.c_str());
        item_current++;
    }
    ImGui::End();
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

    root_scene = new root;
    root_scene->init_root_scene(root_scene);

    scene* scene_test = new scene;
    scene_test->create_new_scene("test_scene", scene_test, root_scene);

    editor_camera = new camera;
    editor_camera->pitch = 0.0f;
    editor_camera->yaw = 0.0f;

    editor_camera->position = glm::vec3(0.0f, 0.0f, 0.0f);

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

    ImGui::SliderFloat("Fov", &fov, 20.f, 110.f);

    camera_view = glm::lookAt(editor_camera->position, get_look_at(editor_camera->position, editor_camera->yaw, editor_camera->pitch), glm::vec3(0, 1, 0));
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

    gizmo_transform_create(camera_view, camera_projection, object_matrix, (float*)identity_matrix, lastUsing, this->editor_camera);

    if (object_matrix.size() > 0)
    {
        for (int matId = 0; matId < object_matrix.size(); matId++)
        {
            ImGuizmo::SetID(matId);

            gizmo_edit_transform(camera_view, camera_projection, object_matrix[matId].data());
            if (ImGuizmo::IsUsing())
            {
                lastUsing = matId;
            }
        }
    }

    gizmo_transform_end();

    ImGui::End();

    object_instance_window();

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