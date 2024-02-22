#pragma once

#include <iostream>
#include <string>
#include <format>
#include <map>
#include <queue>
#include <random>
#include <memory>
#include <tuple>
#include <cmath>
#include <array>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_rect.h>
#include <SDL_render.h>

#include <gffn_exception.h>
#include <gffn_utils.h>
#include <gffn_animation.h>
#include <gffn_events.h>
#include <gffn_physics.h>

namespace gffn {


typedef enum : int {
	GFFN_GENERIC_WIZARD = 0,
	GFFN_GOBLIN_1,
	GFFN_CHARACTER_TYPE_END
} GFFN_CharacterType;

typedef struct GFFN_ObjectIDInfo {
	long unsigned int object_id;
	GFFN_ObjectType object_type;
	GFFN_ObjectIDInfo(long unsigned int object_id, GFFN_ObjectType object_type) : object_id(object_id), object_type(object_type) {}
} GFFN_ObjectIDInfo;

class GFFN_IDable {
private: // Only this class gets to call this function because it would be very bad if two objects had the same ID.
	static long unsigned int gen_object_id() {
		static long unsigned int object_id = 0;
		if (object_id == std::numeric_limits<long unsigned int>::max()) {
			throw GFFN_Exception(std::string("Object ID overflow"));
		}
		return object_id++;
	}
protected:
	long unsigned int object_id;
public:
	static constexpr int DEFAULT_SIZE_LENGTH_OF_OBJECT = 100;
	GFFN_IDable() : object_id(gen_object_id()) {}
	long unsigned int get_object_id() const { return object_id; }
};

class GFFN_GameObject;
extern std::array<std::array<std::vector<GFFN_GameObject*>, WORLD_GRID_HEIGHT>, WORLD_GRID_WIDTH> world_grid;

class GFFN_GameObject : public GFFN_IDable {
protected:	
	// Main variables used in rendering any object.
	SDL_Texture* texture = nullptr;
	void set_texture(SDL_Texture* texture) { this->texture = texture; }
	SDL_Texture* shadow_texture = nullptr;
	void set_shadow_texture(SDL_Texture* shadow_texture) { this->shadow_texture = shadow_texture; }
	std::unique_ptr<SDL_Rect> render_rect;
	void set_render_rect(SDL_Rect render_rect) { this->render_rect = std::make_unique<SDL_Rect>(render_rect); }
	std::unique_ptr<SDL_Rect> source_rect;
	void set_source_rect(SDL_Rect source_rect) { this->source_rect = std::make_unique<SDL_Rect>(source_rect); }
	std::unique_ptr<SDL_Rect> shadow_render_rect;
	void set_shadow_render_rect(SDL_Rect shadow_render_rect) { this->shadow_render_rect = std::make_unique<SDL_Rect>(shadow_render_rect); }
	bool hidden = false;
	double height_offset = 0;

	WorldCoordinateInt2D top_left_coords;
	bool _to_remove = false;
	GFFN_ObjectType object_type;
	std::pair<int, int> grid_location;

	void update_render_rect() {
		render_rect->x = (int)top_left_coords.x;
		render_rect->y = (int)top_left_coords.y;
		shadow_render_rect->x = render_rect->x + render_rect->w / 2 - 50;
		shadow_render_rect->y = render_rect->y + render_rect->h - shadow_render_rect->h;
	}
	int get_texture_width(SDL_Texture* texture) {
		int width;
		SDL_QueryTexture(texture, nullptr, nullptr, &width, nullptr);
		return width;
	}
	int get_texture_height(SDL_Texture* texture) {
		int height;
		SDL_QueryTexture(texture, nullptr, nullptr, nullptr, &height);
		return height;
	}
	void remove_from_curr_grid_location() const {
		for (auto it = world_grid[grid_location.first][grid_location.second].begin(); it != world_grid[grid_location.first][grid_location.second].end(); it++) {
			if ((*it)->get_object_id() == object_id) {
				world_grid[grid_location.first][grid_location.second].erase(it);
				return;
			}
		}
	}
public:
	GFFN_GameObject(GFFN_ObjectType object_type, WorldCoordinateInt2D top_left_coords, int width, int height) : top_left_coords(top_left_coords), object_type(object_type) {
		render_rect = std::make_unique<SDL_Rect>(top_left_coords.x, top_left_coords.y, width, height);
		shadow_render_rect = 
			std::make_unique<SDL_Rect>(render_rect->x + render_rect->w / 2 - 50, render_rect->y + height - DEFAULT_SIZE_LENGTH_OF_OBJECT, DEFAULT_SIZE_LENGTH_OF_OBJECT, DEFAULT_SIZE_LENGTH_OF_OBJECT);
	}
	~GFFN_GameObject() {
		remove_from_curr_grid_location();
	}

	// main API used for rendering:
	SDL_Texture* get_texture() { return texture; }
	SDL_Texture* get_shadow_texture() { return shadow_texture; }
	SDL_Rect* get_render_rect() { return render_rect.get(); }
	SDL_Rect* get_source_rect() { return source_rect.get(); }
	SDL_Rect* get_shadow_render_rect() { return shadow_render_rect.get(); }
	double get_height_offset() const { return height_offset; }
	void set_hidden(bool hidden) { this->hidden = hidden; }
	bool get_hidden() const { return hidden; }
	int get_y() const { return render_rect->y + render_rect->h - FLOOR_COORDS_Y_OFFSET; }

	// API used for controlling the object:
	bool to_remove() const { return _to_remove; }
	void remove() { _to_remove = true; }
	GFFN_ObjectType get_object_type() const { return object_type; }

	// Misc useful APIs:
	WorldCoordinateInt2D get_top_left_coords() const { return top_left_coords; }
	WorldCoordinate get_center_coords() const {
		WorldCoordinate center_coords;
		center_coords.x = top_left_coords.x + ((double)render_rect->w / 2);
		center_coords.y = top_left_coords.y + ((double)render_rect->h / 2);
		return center_coords;
	}
	void set_floor_coords(WorldCoordinate floor_coords) {
		top_left_coords = WorldCoordinateInt2D((int)floor_coords.x - (render_rect->w / 2), (int)floor_coords.y - render_rect->h - FLOOR_COORDS_Y_OFFSET);
	}

	virtual void tick(double delta_time_seconds) {
		update_render_rect();
	};
};

class GFFN_Movable : public GFFN_GameObject {
protected:
	physics::ObjectPhysicsController physics_controller;
public:
	GFFN_Movable(GFFN_ObjectType object_type, int width, int height, WorldCoordinate floor_coords, SDL_Texture* shadow_texture, double ground_coef_friction=7) :
	GFFN_GameObject(object_type, WorldCoordinate(floor_coords.x - (width / 2), floor_coords.y - height - FLOOR_COORDS_Y_OFFSET, 0), width, height),
	physics_controller(floor_coords, ground_coef_friction) {
		// Movable objects have shadows, so init that info here:
		set_shadow_texture(shadow_texture);
		floor_coords = physics_controller.get_floor_coords();
	}

	physics::NormalizedVector3D get_velocity_direction() const {
		return physics::NormalizedVector3D(physics_controller.get_velocity());
	}
	WorldCoordinate get_floor_coords() const {
		return physics_controller.get_floor_coords();
	}
	void set_ground_coef_friction(double ground_coef_friction) {
		physics_controller.set_ground_coef_friction(ground_coef_friction);
	}
	void add_force(physics::Vector3D force) {
		physics_controller.add_force(force);
	}
	bool is_idle() {
		return physics_controller.get_velocity().is_zero();
	}
	bool grounded() {
		return physics_controller.grounded();
	}
	physics::ObjectPhysicsController& get_physics_controller() {
		return physics_controller;
	}

	void tick(double delta_time_seconds) {
		physics_controller.tick(delta_time_seconds);
		height_offset = physics_controller.get_floor_coords().z;
		WorldCoordinate floor_coords = physics_controller.get_floor_coords();
		set_floor_coords(floor_coords);
		GFFN_GameObject::tick(delta_time_seconds);
	}
};

class GFFN_GridObject : public GFFN_Movable {
protected:
	bool object_in_grid_location(std::pair<int, int> loc) const {
		//return world_grid[grid_location.first][grid_location.second].count(object_id) > 0;
		//return std::count(world_grid[grid_location.first][grid_location.second].begin(), world_grid[grid_location.first][grid_location.second].end(), object_id) > 0;

		return loc.first == grid_location.first && loc.second == grid_location.second;
	}
	void update_grid_location_from_floor_coords() {
		WorldCoordinate floor_coords = get_floor_coords();
		int grid_x = (int)(floor_coords.x / 100.0);
		int grid_y = (int)(floor_coords.y / 100.0);
		if (grid_x < 0 || grid_y < 0 || grid_x >= WORLD_GRID_WIDTH || grid_y >= WORLD_GRID_HEIGHT) {
			return;
		}
		if (!object_in_grid_location(std::make_pair(grid_x, grid_y))) {
			// We changed grid!
			remove_from_curr_grid_location();
			world_grid[grid_x][grid_y].push_back(this);
			grid_location.first = grid_x;
			grid_location.second = grid_y;
		}
	}
public:
	GFFN_GridObject(GFFN_ObjectType object_type, int width, int height, WorldCoordinate floor_coords, SDL_Texture* shadow_texture) :
	GFFN_Movable(object_type, width, height, floor_coords, shadow_texture) {}
	~GFFN_GridObject() {
		remove_from_curr_grid_location();
	}
	void tick(double delta_time_seconds) {
		GFFN_Movable::tick(delta_time_seconds);
		update_grid_location_from_floor_coords();
	}
};

class GFFN_Character : public GFFN_GridObject {
protected:
	int size_width_of_character = 100;
	int size_height_of_character = 200;
	static constexpr int SHADOW_HEIGHT_OFFSET = 100;
	static constexpr int SHADOW_WIDTH = 100;
	static constexpr int SHADOW_HEIGHT = 100;
	int hp;
	bool dead = false;
	GFFN_Animations animations;
	physics::NormalizedVector3D look_direction;
public:

	GFFN_Character(GFFN_ObjectType object_type, SDL_Texture* animation_texture, SDL_Texture* shadow_texture,
	int fps, WorldCoordinate floor_coords, int animation_width=25, int animation_height=50) :
	GFFN_GridObject(object_type, animation_width*4, animation_height*4, floor_coords, shadow_texture), animations(animation_texture, fps, animation_width, animation_height), hp(100),
	size_width_of_character(animation_width*4), size_height_of_character(animation_height*4) {
		texture = animation_texture;
		if(floor_coords.x > WORLD_GRID_WIDTH*100) {
			set_floor_coords(WorldCoordinate(WORLD_GRID_WIDTH*100, floor_coords.y, floor_coords.z));
		}
		if (floor_coords.y > WORLD_GRID_HEIGHT*100) {
			set_floor_coords(WorldCoordinate(floor_coords.x, WORLD_GRID_HEIGHT * 99, floor_coords.z));
		}
		if (floor_coords.x < 50) {
			set_floor_coords(WorldCoordinate(100, floor_coords.y, floor_coords.z));
		}
		if (floor_coords.y < 50) {
			set_floor_coords(WorldCoordinate(floor_coords.x, 100, floor_coords.z));
		}
		physics_controller.set_ground_coef_friction(10);
	}
	~GFFN_Character() {}
	void set_animation_state(GFFN_CharacterAnimationState new_animation_state) {
		animations.set_state(new_animation_state);
	}
	bool is_dead() const { return dead; }
	bool change_hp(int hp_change) {
		int previous_hp = hp;
		hp += hp_change;
		if (hp <= 0) hp = 0;
		if (hp > 100) hp = 100;

		if (hp == 0 && previous_hp != 0) {
			dead = true;
		}
		else if (hp != 0 && previous_hp == 0) {
			dead = false;
		}
		return dead;
	}
	void look_at(WorldCoordinate look_at_coords) {
		look_direction = physics::NormalizedVector3D(get_floor_coords(), look_at_coords);
	}
	void animation_tick() {
		animations.tick();
		set_source_rect(*animations.get_current_frame().second);
	}
	void tick(double delta_time_seconds) {
		GFFN_GridObject::tick(delta_time_seconds);
		animation_tick();
	}
};

struct NPC_info {
	GFFN_CharacterType character_type = GFFN_GOBLIN_1;
	WorldCoordinate floor_coords = WorldCoordinate(0, 0, 0);
	SDL_Texture* animation_texture = nullptr;
	SDL_Texture* shadow_texture = nullptr;
	int animation_fps = 5;
	int animation_width = 25;
	int animation_height = 25;
};

class GFFN_NPC : public GFFN_Character {
	typedef enum : int {
		PATROL,
		CHASE,
	} NPCState;

	typedef enum : int {
		WALK_FORWARD,
		WAIT,
	} NPCPatrolState;

	NPC_info npc_info;

	NPCState state = PATROL;

	NPCPatrolState patrol_state = WAIT;
	double state_time_length = 0;
	double time_in_state = 0;

public:
	GFFN_NPC(NPC_info npc_info) :
	GFFN_Character(GFFN_ObjectType::GFFN_OBJECT_TYPE_NPC, npc_info.animation_texture, npc_info.shadow_texture, npc_info.animation_fps, 
	npc_info.floor_coords, npc_info.animation_width, npc_info.animation_height), npc_info(npc_info) {}
	~GFFN_NPC() {}
	void handle_grid_interations() {
		static constexpr double AVOID_CROWDING_CONSTANT_MULTIPLIER = 50.0;
		static constexpr double AVOID_CROWDING_DISTANCE_SQUARED_XY = 4900;
		static constexpr double AVOID_CROWDING_DISTANCE_XY = 70.0;
		static constexpr double PROJECTILE_HIT_DISTANCE = 50.0;
		WorldCoordinate my_coords = get_floor_coords();

		for(int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				if(grid_location.first + i < 0 || grid_location.first + i > WORLD_GRID_WIDTH-1) {
					continue;
				}
				if (grid_location.second + j < 0 || grid_location.second + j > WORLD_GRID_HEIGHT-1) {
					continue;
				}
				auto& grid = world_grid[grid_location.first + i][grid_location.second + j];
				for (GFFN_GameObject* const other_object_base : grid) {
					GFFN_ObjectType other_object_type = other_object_base->get_object_type();
					long unsigned int other_object_id = other_object_base->get_object_id();
					
					GFFN_GridObject * other_object = static_cast<GFFN_GridObject*>(other_object_base);
					if (other_object_id != object_id && (other_object_type == GFFN_OBJECT_TYPE_CHARACTER || other_object_type == GFFN_OBJECT_TYPE_NPC)) {
						WorldCoordinate other_coords = other_object->get_floor_coords();
						//double distance = my_coords.distance_from(other_coords);
						double distance_squared_xy = my_coords.distance_from_squared_xy(other_coords);
						if (distance_squared_xy < AVOID_CROWDING_DISTANCE_SQUARED_XY) {
							try {
								physics::NormalizedVector3D direction_vector(other_coords, my_coords);
								physics::Vector3D force_to_add = physics::Vector3D(direction_vector.x * 50000.0 * (AVOID_CROWDING_DISTANCE_SQUARED_XY / distance_squared_xy),
									direction_vector.y * 50000.0 * (AVOID_CROWDING_DISTANCE_SQUARED_XY / distance_squared_xy), 0.0);
								if(force_to_add.magnitude() > 500000.0) {
									force_to_add = physics::Vector3D(direction_vector.x * 200000.0, direction_vector.y * 200000.0, 0.0);
								}
								physics_controller.add_force(force_to_add);
							}
							catch (std::runtime_error&) {
								physics_controller.add_force(physics::Vector3D(1, 1, 0));
							}
						}
					}
				}
			}
		}
	}
	void patrol_tick(double delta_time_seconds) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_real_distribution<> rand_deg(0, 360);
		static std::uniform_real_distribution<> rand_time(2, 3);

		time_in_state += delta_time_seconds;

		if (time_in_state > state_time_length) {
			time_in_state = 0;
			state_time_length = rand_time(gen);
			if (patrol_state == WALK_FORWARD) {
				patrol_state = WAIT;
				//set_velocity(GFFN_Velocity(0.0, 0.0));
			}
			else {
				look_direction.rotate_xy(rand_deg(gen));
				patrol_state = WALK_FORWARD;
			}
		}
		else {
			if(patrol_state == WALK_FORWARD)
				physics_controller.add_force(physics::Vector3D(look_direction.x * 100000.0, look_direction.y * 100000.0, 0.0));
		}
	}
	void tick(double delta_time_seconds) {
		handle_grid_interations();
		GFFN_Character::tick(delta_time_seconds);

		if (dead) {
			return;
		}
		this->animations.set_state(0);

		switch (state) {
		case PATROL: {
			patrol_tick(delta_time_seconds);
			break;
		}
		default: {
			break;
		}
		}

	}
};

class GFFN_StraightProjectile : public GFFN_GridObject {
	double life_time_seconds;
	double time_alive = 0;
	physics::Vector3D propulsion_force;
	WorldCoordinate dest_coords;
public:
	static constexpr int SIZE_LENGTH_OF_OBJECT = 100;
	GFFN_StraightProjectile(WorldCoordinate start_coords, physics::NormalizedVector3D direction_vector, double propulsion_force_magnitude,
		double life_time_seconds, SDL_Texture* texture, SDL_Texture* shadow_texture) :
		GFFN_GridObject(GFFN_ObjectType::GFFN_OBJECT_TYPE_STRAIGHT_PROJECTILE, SIZE_LENGTH_OF_OBJECT, SIZE_LENGTH_OF_OBJECT, start_coords, shadow_texture), life_time_seconds(life_time_seconds) {
		physics::Vector3D initial_velocity(direction_vector.x * propulsion_force_magnitude * 0.3, direction_vector.y * propulsion_force_magnitude * 0.3, -propulsion_force_magnitude);
		physics_controller.set_velocity(initial_velocity);
		propulsion_force = physics::Vector3D(direction_vector.x * propulsion_force_magnitude, direction_vector.y * propulsion_force_magnitude, 0.0);
		set_ground_coef_friction(0.5);
		physics_controller.set_mass(10);
		set_texture(texture);
	}
	void tick(double delta_time_seconds) {
		WorldCoordinate floor_coords = get_floor_coords();
		if (floor_coords.z < 40) {
			physics_controller.set_height(41.0);
		}
		physics_controller.add_force(propulsion_force);

		GFFN_GridObject::tick(delta_time_seconds);
		time_alive += delta_time_seconds;
		if (time_alive > life_time_seconds) {
			remove();
		}
		else {
			for (auto it = world_grid[grid_location.first][grid_location.second].begin(); it != world_grid[grid_location.first][grid_location.second].end(); it++) {
				if ((*it)->get_object_type() == GFFN_OBJECT_TYPE_NPC) {
					GFFN_Character* character = static_cast<GFFN_Character*>(*it);
					WorldCoordinate character_coords = character->get_floor_coords();
					character_coords.z += (double)character->get_render_rect()->h / 2;
					double distance = floor_coords.distance_from(character_coords);
					if (distance < 50) {
						events::ProjectileHitEvent projectile_hit_event(this, character);
						events::event_queue.push(projectile_hit_event);
						remove_from_curr_grid_location();
						character->change_hp(get_damage() * -1);
						character->get_physics_controller().transfer_momentum((this->get_physics_controller()));
						remove();
						break;
					}
				}
			}
		}
	}
	int get_damage() {
		return 34; // temp
	}
};

class GFFN_UIObject : public GFFN_GameObject {
	static constexpr int SIZE_LENGTH_OF_OBJECT = 100;
public:
	GFFN_UIObject(SDL_Renderer* renderer, SDL_Texture* texture, WorldCoordinate top_left_coords) :
	GFFN_GameObject(GFFN_ObjectType::GFFN_OBJECT_TYPE_UI_OBJECT, top_left_coords, SIZE_LENGTH_OF_OBJECT, SIZE_LENGTH_OF_OBJECT) {
		set_texture(texture);
	}
	~GFFN_UIObject() {}
	SDL_Texture* get_texture() { return texture; }
	void tick(double delta_time_seconds) {}
};

class GFFN_EnvironmentalObject : public GFFN_GridObject {
public:
	GFFN_EnvironmentalObject(WorldCoordinate floor_coords, SDL_Texture* texture, SDL_Texture* shadow_texture) :
	GFFN_GridObject(GFFN_ObjectType::GFFN_OBJECT_TYPE_ENVIRONMENTAL_OBJECT, get_texture_width(texture)*4, get_texture_height(texture)*4, floor_coords, shadow_texture) {
		set_texture(texture);
		GFFN_GridObject::tick(0);
	}
	~GFFN_EnvironmentalObject() {}
	void tick(double delta_time_seconds) {}
};

class GFFN_GroundObject : public GFFN_GameObject {
public:
	// Inanimate objects are 1m x 1m, and their coordinates have to align with the grid, so the x and y coordinates are in meters for the constructor.
	GFFN_GroundObject(SDL_Texture* texture) :
	GFFN_GameObject(GFFN_ObjectType::GFFN_OBJECT_TYPE_GROUND_OBJECT, WorldCoordinateInt2D(0, 0), WORLD_GRID_WIDTH*100, WORLD_GRID_HEIGHT*100) {
		set_texture(texture);
	}
	~GFFN_GroundObject() {}
	void tick(double delta_time_seconds) {}
};

class GFFN_DismemberedBodyPart : public GFFN_GridObject {
public:
	GFFN_DismemberedBodyPart(WorldCoordinate floor_coords, SDL_Texture* texture, SDL_Texture* shadow_texture, SDL_Rect* source_rect) :
	GFFN_GridObject(GFFN_ObjectType::GFFN_OBJECT_TYPE_DISMEMBERED_BODY_PART, 100, 100, floor_coords, nullptr) {
		set_texture(texture);
		set_source_rect(*source_rect);
	}
	~GFFN_DismemberedBodyPart() {}
	void tick(double delta_time_seconds) {
		GFFN_GridObject::tick(delta_time_seconds);
	}
};

typedef std::vector<std::pair<int, std::unique_ptr<GFFN_GameObject>>> VectorOfObjectsByY;

} // end namespace gffn