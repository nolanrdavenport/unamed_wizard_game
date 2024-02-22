#pragma once

#include <vector>
#include <map>
#include <queue>
#include <random>
#include <array>

#include <gffn_game_object.h>

namespace gffn {

typedef std::vector<std::pair<int, GFFN_GameObject*>> objects_by_y_t;
typedef long unsigned int object_id_t;

// This class will hold all game objects that are to be rendered.
class GameWorldObjects {
	std::unordered_map<object_id_t, std::unique_ptr<GFFN_GameObject>> game_objects;
	std::vector<GFFN_GameObject*> game_objects_vec;

	// this will be used for rendering, it has to be sorted after being filled.
	std::vector<std::pair<int, GFFN_GameObject*>> objects_by_y;
public:
	GameWorldObjects() {}
	~GameWorldObjects() {}

	bool object_exists(object_id_t object_id) const {
		return game_objects.count(object_id) == 1;
	}

	void add_object(std::unique_ptr<GFFN_GameObject> object) {
		object_id_t object_id = object->get_object_id();
		game_objects[object_id] = std::move(object);
		game_objects_vec.push_back(game_objects[object_id].get());
	}

	void remove_object(object_id_t object_id) {
		for (auto it = game_objects_vec.begin(); it != game_objects_vec.end(); ++it) {
			if ((*it)->get_object_id() == object_id) {
				game_objects_vec.erase(it);
				break;
			}
		}
		game_objects.erase(object_id);
	}

	GFFN_GameObject* const get_object(object_id_t object_id) const {
		return game_objects.at(object_id).get();
	}

	std::vector<GFFN_GameObject*>::iterator remove_object(std::vector<GFFN_GameObject*>::iterator it) {
		game_objects.erase((*it)->get_object_id());
		return game_objects_vec.erase(it);
	}

	std::vector<GFFN_GameObject*>::iterator begin() { return game_objects_vec.begin(); }
	std::vector<GFFN_GameObject*>::iterator end() { return game_objects_vec.end(); }

	void clear_objects_by_y() { objects_by_y.clear(); }
	void add_to_objects_by_y(GFFN_GameObject* object) {
		objects_by_y.emplace_back(object->get_y(), object);
	}
	void sort_objects_by_y() { std::sort(objects_by_y.begin(), objects_by_y.end()); }
	objects_by_y_t & get_objects_by_y() { return objects_by_y; }
};

} // end namespace gffn