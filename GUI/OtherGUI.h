/* GUI stuff */
void object_instance_window() {
    ImGui::SetNextWindowPos(ImVec2(1110, 10));
    ImGui::SetNextWindowSize(ImVec2(30, 400));
    ImGui::Begin("Instances");
    static std::vector<char *> items;
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

void mesh_selector() {
    ImGui::Separator();
    ImGui::Text("Model");
    ImGui::Button("Select Mesh");
    ImGui::SameLine();
    ImGui::Text("Load another .OBJ Model:");
}