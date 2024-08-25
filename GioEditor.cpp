#include "GioEditor.h"
#include "Camera/Camera.h"
#include "GUI/GizmoGUI.h"
#include "GUI/Style.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "GameObject.h"

#include <ImGuizmo.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <filesystem>
#include <iostream>

/* constructors */
GioEditor::GioEditor() {
    this->screen_width = 1280;
    this->screen_height = 720;
}

camera* GioEditor::get_cam() {
    return editor_camera;
}

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

/*

Sky Box code!!

*/

std::array<std::string, 6> cubemap;

unsigned int get_cubemap_texture_id();

/*

    Example:
        load_cubemap(
            "right.jpg",
            "left.jpg",
            "top.jpg",
            "bottom.jpg",
            "front.jpg",
            "back.jpg"
        );

*/

unsigned int load_cubemap(const char* right, const char* left, const char* top, const char* bottom, const char* front, const char* back) {
    cubemap[0] = right;
    cubemap[1] = left;
    cubemap[2] = top;
    cubemap[3] = bottom;
    cubemap[4] = front;
    cubemap[5] = back;

    return get_cubemap_texture_id();
}

unsigned int get_cubemap_texture_id() {
    unsigned int texture_id;
    glGenTextures(1, &texture_id);


    int width, height, nrChannels;
    for (unsigned int i = 0; i < 6; i++) {
        unsigned char *data = stbi_load(cubemap[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap tex Failed to load at path: " << cubemap[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return texture_id;
}

std::vector<mesh> meshes;
std::vector<mesh*> objects;

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

// TODO: Move this to another file, maybe a console?
/* Console */
std::vector<char*> console_items;
static int console_item_current = 0;

void console_window() {
    ImGui::SetNextWindowPos(ImVec2(10, 420));
    ImGui::SetNextWindowSize(ImVec2(1260, 290));
    ImGui::Begin("Console", 0);
    ImGui::ListBox(" ", &console_item_current, console_items.data(), console_items.size(), 10);
    ImGui::End();
}

void push_to_console(std::string string) {
    std::string* str = new std::string(string);
    console_items.push_back((char*)str->c_str());
    console_item_current++;
}

/* == */

// TODO: Move this to another file, ObjLoder?
/* Assimp importer stuff*/
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

            //node* current_scene = get_node_from_id(root_scene->current_scene);
            //mesh* new_object = (mesh*)create_new_object("mesh_object", "mesh");
            //current_scene->add_child(new_object);
            //object_matrix.push_back(new_object->transform);

            // Process the root node
            processNode(scene->mRootNode, scene);

            //push_to_console("Loaded new mesh: " + new_object->name + " created with: " + std::to_string(new_object->numVertices) + " vertices");
        }
    }
}

static int item_current = 0;

/* == */

static int open_popup = false;

void update_mesh(glm::mat4 view, glm::mat4 projection, int window_width, int window_height, int lastUsing);
void render_precomputed_mesh();

int mesh_select_popup()
{
    int selected = -1;
    if (ImGui::BeginPopup("meshpopup")) // <-- use last item id as popup id
    {
        // Get the size of the child window for rendering the mesh
        ImVec2 childSize = ImVec2(100, 100); // Customize as needed

        int count = 0;

        for (mesh static_mesh : meshes) {
            count++;

            ImVec2 relative_pos = ImGui::GetCursorPos();

            // Set cursor position and begin the child window
            ImGui::SetCursorPos(relative_pos);
            ImGui::BeginChild(("mesh" + std::to_string(count)).c_str(), childSize, true);

            ImVec2 childPos = ImGui::GetCursorScreenPos();
            int viewportX = static_cast<int>(childPos.x);
            int viewportY = static_cast<int>(childPos.y);
            int viewportWidth = static_cast<int>(childSize.x);
            int viewportHeight = static_cast<int>(childSize.y);

            glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
            glScissor(viewportX, viewportY, viewportWidth, viewportHeight);

            glm::mat4 viewMatrix = glm::lookAt(
                glm::vec3(0.0f, 0.0f, 3.0f),  // Camera position
                glm::vec3(0.0f, 0.0f, 0.0f),  // Look at point
                glm::vec3(0.0f, 1.0f, 0.0f)   // Up vector
            );

            glm::mat4 projectionMatrix = glm::perspective(
                glm::radians(45.0f),          // Field of view
                1.0f,                         // Aspect ratio (adjust if needed)
                0.1f,                         // Near plane
                100.0f                        // Far plane
            );

            update_mesh(viewMatrix, projectionMatrix, viewportWidth, viewportHeight, count - 1);
            render_precomputed_mesh();

            if (ImGui::Button("select")) {
                selected = count - 1;

                ImGui::CloseCurrentPopup();
                open_popup = false;
            }

            ImGui::EndChild();

            ImGui::SameLine();
        }


        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
            open_popup = false;
        }
        ImGui::EndPopup();
    }
    ImGui::OpenPopup("meshpopup");

    return selected;
}

// TODO: Move to GUI/Something
// TODO: Gizmo selected should be directly linked to what is selected here
/* GUI stuff */
void object_instance_window(int *item_current) {
    ImGui::SetNextWindowPos(ImVec2(1110, 10));
    ImGui::SetNextWindowSize(ImVec2(30, 400));
    ImGui::Begin("Instances");
    static std::vector<char *> items;
    ImGui::ListBox(" ", item_current, items.data(), items.size(), 10);
    ImGui::SameLine();
    if (ImGui::Button("+", ImVec2(20, 20))) {
        node* current_scene = get_node_from_id(root_scene->current_scene);
        mesh* new_object = (mesh*)create_new_object("mesh_object", "mesh");
        current_scene->add_child(new_object);
        object_matrix.push_back(new_object->transform);
        items.push_back((char*)new_object->name.c_str());
        *item_current++;

        push_to_console("Added instance " + new_object->name);

        mesh* m_object = new mesh();
        m_object->numVertices = meshes[0].numVertices;
        m_object->vertices = meshes[0].vertices;

        objects.push_back(m_object);
    }
    ImGui::End();
}

void load_object_mesh(mesh *m_object, int current_mesh) {
    m_object->numVertices = meshes[current_mesh].numVertices;
    m_object->vertices = meshes[current_mesh].vertices;
}

void mesh_selector() {
    ImGui::Separator();
    ImGui::Text("Model");
    if (ImGui::Button("Select Mesh")) open_popup = true;
    ImGui::SameLine();
    ImGui::Text("Load another .OBJ Model:");
}

std::vector<ImVec2> precomputedVertices;

void precompute_static_mesh(glm::mat4 model, glm::mat4 view, glm::mat4 projection, int window_width, int window_height, int meshIndex) {
    const mesh& mesh = meshes[meshIndex];
    glm::mat4 modelViewProjection = projection * view * model;

    ImVec2 windowPos = ImGui::GetWindowPos();

    precomputedVertices.clear();  // Clear previous data
    precomputedVertices.reserve(mesh.numVertices);  // Allocate memory

    for (unsigned int i = 0; i < mesh.numVertices; i++) {
        glm::vec4 vertexClip = modelViewProjection * glm::vec4(mesh.vertices[i], 1.0f);

        if (vertexClip.w > 0.0f) {
            glm::vec3 vertexNDC = glm::vec3(vertexClip) / vertexClip.w;
            float x = (vertexNDC.x + 1.0f) * 0.5f * window_width + windowPos.x;
            float y = (1.0f - vertexNDC.y) * 0.5f * window_height + windowPos.y;
            precomputedVertices.push_back(ImVec2(x, y));
        }
    }
}

void update_mesh(glm::mat4 view, glm::mat4 projection, int window_width, int window_height, int lastUsing) {
    static glm::mat4 lastView;
    static glm::mat4 lastProjection;
    static int lastLastUsing;

    if (view != lastView || projection != lastProjection || lastLastUsing != lastUsing) {
        precompute_static_mesh(glm::mat4(1.0f), view, projection, window_width, window_height, lastUsing);
        lastView = view;
        lastProjection = projection;
        lastLastUsing = lastUsing;
    }
}

void render_precomputed_mesh() {
    ImDrawList* list = ImGui::GetWindowDrawList();

    for (size_t i = 0; i < precomputedVertices.size(); i += 3) {
        list->AddConvexPolyFilled(&precomputedVertices[i], 3, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
    }
}

// Function to compile a shader
unsigned int compile_shader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

// Create and link the shader program
unsigned int create_shader_program(unsigned int vertex_shader, unsigned int frament_shader) {
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex_shader);
    glAttachShader(shaderProgram, frament_shader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete the shaders as they're linked into the program now and no longer needed
    glDeleteShader(vertex_shader);
    glDeleteShader(frament_shader);

    return shaderProgram;
}

unsigned int skybox_shader;

void load_skybox_shader() {
    const char *vertex_shader_str = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;

        out vec3 TexCoords;

        uniform mat4 projection;
        uniform mat4 view;

        void main()
        {
            TexCoords = aPos;
            gl_Position = projection * view * vec4(aPos, 1.0);
        }  
    )";

    const char *fragment_shader_str = R"(
        #version 330 core
        out vec4 FragColor;

        in vec3 TexCoords;

        uniform samplerCube skybox;

        void main()
        {    
            FragColor = texture(skybox, TexCoords);
        }
    )";

    unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_str);
    unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_str);

    skybox_shader = create_shader_program(vertex_shader, fragment_shader);
    glUseProgram(skybox_shader);
}

unsigned int cubemap_texture;
unsigned int skybox_vao;

int GioEditor::ready() {
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


    /*
    
        Cubemap code!!

    */
   
    float skybox_vertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    unsigned int skybox_vbo;
    glGenVertexArrays(1, &skybox_vao);
    glGenBuffers(1, &skybox_vbo);
    glBindVertexArray(skybox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    unsigned int cubemap_texture = load_cubemap(
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"
    );

    /*
        skybox shader usage!!
    */

    load_skybox_shader();
    glUniform1i(glGetUniformLocation(skybox_shader, "skybox"), 0);

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

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    // create a window and insert the inspector
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(320, 400));

    ImGui::Begin("Editor");

    //Perspective(fov, io.DisplaySize.x / io.DisplaySize.y, 0.1f, 100.f, cameraProjection);
    float aspect = io.DisplaySize.x / io.DisplaySize.y;
    if (aspect < std::numeric_limits<float>::epsilon()) {
        aspect = 1.0f; // Avoid zero aspect ratio
    }
    camera_projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

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

    gizmo_transform_create(camera_view, camera_projection, object_matrix, (float*)identity_matrix, *lastUsing, this->editor_camera, objects);

    if (object_matrix.size() > 0)
    {
        int matId = 0;
        for (const mesh* mesh : objects) {
            ImGuizmo::SetID(matId);
            gizmo_edit_transform(camera_view, camera_projection, glm::value_ptr(object_matrix[matId]));
            if (ImGuizmo::IsUsing())
            {
                *lastUsing = matId;
            }
            matId++;
        }
    }

    gizmo_transform_end();

    if (object_matrix.size() > 0)
    {
        mesh_selector();

        ImGui::BeginChild("idk, test");
        update_mesh(glm::lookAt(glm::vec3(1.0f, 0.5f, 2.5f), get_look_at(glm::vec3(0.0f), 0.0, 0.0), glm::vec3(0, 1, 0)), glm::perspective(20.0f, 1.0f, 0.1f, 100.0f), 100, 100, objects[*lastUsing]->current_mesh);
        render_precomputed_mesh();
        ImGui::EndChild();
    }

    ImGui::End();

    object_instance_window(lastUsing);
    console_window();

    if (open_popup) {
        int current_mesh = mesh_select_popup();
        if (current_mesh != -1) {
            load_object_mesh(objects[*lastUsing], current_mesh);
            objects[*lastUsing]->current_mesh = current_mesh;
        }
    }

    ImGui::SetNextWindowPos(ImVec2(10, 350));
    ImGui::SetNextWindowSize(ImVec2(940, 480));

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(main_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    // rendering
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    /*

        Skybox Rendering

    */

   // TODO: use framebuffers to render the gizmo
   /*{
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        glUseProgram(skybox_shader);
        glm::mat4 view = glm::mat4(glm::mat3(camera_view)); // remove translation from the view matrix
        //glm::mat4 projection = glm::mat4(glm::mat3(camera_projection));
        glUniformMatrix4fv(glGetUniformLocation(skybox_shader, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(skybox_shader, "projection"), 1, GL_FALSE, &camera_projection[0][0]);

        // skybox cube
        glBindVertexArray(skybox_vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);

        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default
    }*/

    // before finishing current frame
    glfwSwapBuffers(main_window);
    glfwPollEvents();

    return 0;
}

int GioEditor::clean_up() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(this->main_window);
    glfwTerminate();

    return 0;
}