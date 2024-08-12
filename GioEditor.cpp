#include "GioEditor.h"
#include "Camera/Camera.h"
#include "GUI/GizmoGUI.h"
#include "GUI/Style.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "GameObject.h"

#include <ImGuizmo.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <filesystem>
#include <iostream>

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

std::vector<mesh> meshes;

// objects matrices
std::vector<glm::mat4> object_matrix;

static const float identity_matrix[16] =
{ 1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 1.f, 0.f,
    0.f, 0.f, 0.f, 1.f };

// Camera controls
glm::vec3 old_mouse_pos;

// scene creation
root* root_scene;

std::vector<char*> console_items;
static int console_item_current = 0;

// Load the model using Assimp
Assimp::Importer importer;

void processNode(aiNode* node, const aiScene* scene);
void processMesh(aiMesh* mesh, const aiScene* /*scene*/);

void processNode(aiNode* node, const aiScene* scene) {
    // Process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }

    // Then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}


void processMesh(aiMesh* ai_mesh, const aiScene* /*scene*/) {
    mesh m;

    // Iterate through all vertices in the aiMesh
    for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++) {
        glm::vec3 vertex;
        vertex.x = ai_mesh->mVertices[i].x;
        vertex.y = ai_mesh->mVertices[i].y;
        vertex.z = ai_mesh->mVertices[i].z;

        // Add the vertex to the mesh's vertex list
        m.vertices.push_back(vertex);
    }

    // Store the number of vertices
    m.numVertices = ai_mesh->mNumVertices;

    // Push the mesh into the vector of meshes
    meshes.push_back(m);
}

void push_to_console(std::string string)
{
    std::string* str = new std::string(string);
    console_items.push_back((char*)str->c_str());
    console_item_current++;
}

void loadAllObjFiles(const std::string& directory) {
    Assimp::Importer importer;

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".obj") {
            const aiScene* scene = importer.ReadFile(entry.path().string(),
                aiProcess_Triangulate | aiProcess_FlipUVs);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cerr << "Failed to load model: " << entry.path().string() << " - "
                    << importer.GetErrorString() << std::endl;
                continue;
            }

            node* current_scene = get_node_from_id(root_scene->current_scene);
            mesh* new_object = (mesh*)create_new_object("mesh_object", "mesh");
            current_scene->add_child(new_object);
            object_matrix.push_back(new_object->transform);

            // Process the root node
            processNode(scene->mRootNode, scene);

            push_to_console("Loaded new mesh: " + new_object->name + " created with: " + std::to_string(new_object->numVertices) + " vertices");
        }
    }
}

// GUI stuff
void object_instance_window()
{
    ImGui::SetNextWindowPos(ImVec2(1110, 10));
    ImGui::SetNextWindowSize(ImVec2(30, 400));
    ImGui::Begin("Instances");
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

        push_to_console("Added instance " + new_object->name);

        const aiScene* scene = importer.ReadFile("cube.obj",
            aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            push_to_console("Failed to load model: " + std::string(importer.GetErrorString()));
            return;
        }

        // Process the root node
        processNode(scene->mRootNode, scene);
    }
    ImGui::End();
}

void console_window()
{
    ImGui::SetNextWindowPos(ImVec2(10, 420));
    ImGui::SetNextWindowSize(ImVec2(1260, 290));
    ImGui::Begin("Console", 0);
    ImGui::ListBox(" ", &console_item_current, console_items.data(), console_items.size(), 10);
    ImGui::End();
}

void mesh_selector()
{
    ImGui::Separator();
    ImGui::Text("Model");
    ImGui::Button("Select Mesh");
    ImGui::SameLine();
    ImGui::Text("Load another .OBJ Model:");
}

void render_mesh(glm::mat4 camera_view, glm::mat4 camera_projection, int window_width, int window_height, int lastUsing)
{
    ImDrawList* list = ImGui::GetWindowDrawList();

    const mesh& mesh = meshes[lastUsing];
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera_view;
    glm::mat4 projection = camera_projection;

    std::vector<ImVec2> triangleVertices;
    for (unsigned int i = 0; i < mesh.numVertices; i++) {
        // Convert each vertex from model space to world space
        glm::vec4 vertexWorld = model * glm::vec4(mesh.vertices[i], 1.0f);
        // Then from world space to clip space
        glm::vec4 vertexClip = projection * view * vertexWorld;

        if (vertexClip.w > 0.0f) {  // Check to avoid divide by zero
            glm::vec3 vertexNDC = glm::vec3(vertexClip) / vertexClip.w;
            // Convert NDC to screen space
            float x = (vertexNDC.x + 1.0f) * 0.5f * window_width + ImGui::GetWindowPos().x;
            float y = (1.0f - vertexNDC.y) * 0.5f * window_height + ImGui::GetWindowPos().y;
            triangleVertices.push_back(ImVec2(x, y));
        }

        // When we've collected 3 vertices (1 triangle), draw it
        if (triangleVertices.size() == 3) {
            list->AddConvexPolyFilled(triangleVertices.data(), 3, ImGui::GetColorU32(ImVec4(0.32f, 0.32f, 0.32f, 1.0f)));
            triangleVertices.clear();  // Clear the vector to prepare for the next triangle
        }
    }
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

    //auto font_default = io.Fonts->AddFontDefault();
    ImFont* font1 = io.Fonts->AddFontFromFileTTF("Ubuntu-Light.ttf", 14);

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

    push_to_console("Called root_scene->init_root_scene()");

    scene* scene_test = new scene;
    scene_test->create_new_scene("test_scene", scene_test, root_scene);

    push_to_console("Created new scene " + scene_test->name);

    editor_camera = new camera;
    editor_camera->pitch = 0.0f;
    editor_camera->yaw = 0.0f;

    editor_camera->position = glm::vec3(0.0f, 0.0f, 0.0f);

    loadAllObjFiles(".");

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

    gizmo_transform_create(camera_view, camera_projection, object_matrix, (float*)identity_matrix, lastUsing, this->editor_camera, meshes);

    if (object_matrix.size() > 0)
    {
        int matId = 0;
        for (const mesh& mesh : meshes) {
            ImGuizmo::SetID(matId);
            gizmo_edit_transform(camera_view, camera_projection, glm::value_ptr(object_matrix[matId]));
            if (ImGuizmo::IsUsing())
            {
                lastUsing = matId;
            }
            matId++;
        }
    }

    gizmo_transform_end();

    if (object_matrix.size() > 0)
    {
        mesh_selector();

        //ImGuiWindowFlags window_flags = ImGuiWindowFlags

        ImGui::BeginChild("idk, test");
        render_mesh(glm::lookAt(glm::vec3(1.0f, 0.5f, 2.5f), get_look_at(glm::vec3(0.0f), 0.0, 0.0), glm::vec3(0, 1, 0)), glm::perspective(20.0f, 1.0f, 0.1f, 100.0f), 100, 100, lastUsing);
        ImGui::EndChild();
    }

    ImGui::End();

    object_instance_window();
    console_window();

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