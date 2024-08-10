#include "GameObject.h"
#include <sstream>

void node::add_child(node* child)
{
	children_ids.push_back(child->id);
	child->parent_id = this->id;
}

void root::init_root_scene(root *root)
{
	std::stringstream root_id;
	root_id << root;
	root->id = root_id.str();
}

std::string root::get_id()
{
	return this->id;
}

void scene::create_new_scene(std::string name, scene *scene, root *root)
{
	std::stringstream scene_id;
	scene_id << scene;
	scene->id = scene_id.str();

	scene->name = name;
	scene->parent_id = root->get_id();

	root->scenes.push_back(scene->id);
	root->current_scene = scene->id;
}

node* create_new_object(std::string name, std::string type)
{
	if (type == "mesh") {
		mesh* temp_obj = new mesh;

		std::stringstream object_id;
		object_id << temp_obj;
		temp_obj->id = object_id.str();

		temp_obj->name = name;

		temp_obj->transform = {
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			2.f, 0.f, 2.f, 1.f
		};

		return (node*)temp_obj;
	}

	return nullptr;
}

node* get_node_from_id(std::string id)
{
	std::stringstream ss;
	void* ptrBack;
	ss.clear();
	ss.str(id);
	ss >> ptrBack;

	return static_cast<node*>(ptrBack);
}