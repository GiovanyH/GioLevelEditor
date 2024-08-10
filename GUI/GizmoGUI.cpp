#include "GizmoGUI.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include "CameraControls.h"
#include <ImGuizmo.h>

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
void idle_camera_mode()
{
    if (ImGui::IsKeyPressed(ImGuiKey_W))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E)) // r Key
        mCurrentGizmoOperation = ImGuizmo::SCALE;
}

int get_key_down(ImGuiKey key)
{
    return ImGui::IsKeyDown(key);
}

// Gizmo manipulation
void gizmo_transform_create(glm::mat4 camera_view, glm::mat4 camera_projection, std::vector<std::array<float, 16>> object_matrix, float* identity_matrix, int lastUsing, camera* editor_camera)
{
    static float old_pitch;
    static float old_yaw;
    static bool camera_mode = false;

    // Get the GLFW window
    GLFWwindow* glfw_window = glfwGetCurrentContext();
    // Get the window size
    static float window_width;
    static float window_height;

    // Calculate the center of the window
    glm::vec2 screen_center(window_width / 2.0f, window_height / 2.0f);

    ImGuiIO& io = ImGui::GetIO();

    camera* first_camera = editor_camera;

    float* matrix;

    if (object_matrix.size() > 0) matrix = object_matrix[lastUsing].data();
    else matrix = nullptr;

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        // Reset the mouse position to the center
        glfwSetCursorPos(glfw_window, screen_center.x, screen_center.y);
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        camera_mode = true;
    }
    else {
        camera_mode = false;
    }

    if (camera_mode) fps_camera_control(first_camera, get_direction_based_on_keys(ImGui::IsKeyDown(ImGuiKey_W), ImGui::IsKeyDown(ImGuiKey_S), ImGui::IsKeyDown(ImGuiKey_D), ImGui::IsKeyDown(ImGuiKey_A), ImGui::IsKeyDown(ImGuiKey_LeftShift), ImGui::IsKeyDown(ImGuiKey_LeftCtrl)), window_width, window_height);
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
    if (object_matrix.size() > 0) {
        ImGuizmo::DecomposeMatrixToComponents(matrix, matrix_translation, matrix_rotation, matrix_scale);
        ImGui::InputFloat3("Tr", matrix_translation);
        ImGui::InputFloat3("Rt", matrix_rotation);
        ImGui::InputFloat3("Sc", matrix_scale);
        ImGuizmo::RecomposeMatrixFromComponents(matrix_translation, matrix_rotation, matrix_scale, matrix);
    }

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
    static ImGuiWindowFlags gizmo_window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(330, 10), ImGuiCond_Appearing);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
    ImGui::Begin("Gizmo", 0, gizmo_window_flags);
    ImGuizmo::SetDrawlist();
    window_width = (float)ImGui::GetWindowWidth();
    window_height = (float)ImGui::GetWindowHeight();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, window_width, window_height);
    view_manipulate_right = ImGui::GetWindowPos().x + window_width;
    view_manipulate_top = ImGui::GetWindowPos().y;

    ImGuizmo::DrawGrid((const float*)glm::value_ptr(camera_view), (const float*)glm::value_ptr(camera_projection), identity_matrix, 100.f);
    //                                                                                                             for now \/
    // Shitty fucking code below \/
    if (object_matrix.size() > 0)
    {
        std::vector<float> flat_matrices;
        flat_matrices.reserve(object_matrix.size() * 16);

        // Flatten the matrices
        for (const auto& matrix : object_matrix) {
            flat_matrices.insert(flat_matrices.end(), matrix.begin(), matrix.end());
        }
        ImGuizmo::DrawCubes((const float*)glm::value_ptr(camera_view), (const float*)glm::value_ptr(camera_projection), (const float*)&(flat_matrices.data()[0]), object_matrix.size());
    }
    ImGuizmo::ViewManipulate((float*)glm::value_ptr(camera_view), 1.0f, ImVec2(view_manipulate_right - 128, view_manipulate_top), ImVec2(128, 128), 0x10101010);
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