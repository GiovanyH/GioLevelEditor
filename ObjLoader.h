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

static int item_current = 0;