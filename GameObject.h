#pragma once
#include <string>
#include <vector>
#include <array>

#include <glm/glm.hpp>

class node
{
public:
	std::string parent_id;
	std::vector<std::string> children_ids;
	std::string id;
	std::string name;

	glm::mat4 transform;

	void add_child(node* child);
};

class root
{
private:
	std::string id;
public:
	std::vector<std::string> scenes;
	std::string current_scene;

	void init_root_scene(root *root);
	std::string get_id();
};

class scene : public node
{
public:
	void create_new_scene(std::string name, scene* scene, root* root);
};

class mesh : public node
{
public:
	unsigned int numVertices;
	std::vector<glm::vec3> vertices;
};

node* create_new_object(std::string name, std::string type);
node* get_node_from_id(std::string id);