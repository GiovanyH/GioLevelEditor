
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