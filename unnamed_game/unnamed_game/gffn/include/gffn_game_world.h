#pragma once

#include <SDL.h>
#include <SDL_render.h>
#include <SDL_image.h>

#include <gffn_camera.h>
#include <gffn_utils.h>
#include <gffn_renderer.h>
#include <gffn_events.h>
#include <gffn_game_world_objects.h>

#include <string>

#include <vector>
#include <map>
#include <queue>
#include <random>
#include <array>
#include <algorithm>

namespace gffn {

class GFFN_GameWorld {
public:
	int num_characters = 0;
	int num_npcs = 0;
	int num_ui_objects = 0;
	int num_dismembered_body_parts = 0;
	int num_environmental_objects = 0;
	int num_straight_projectiles = 0;


	GFFN_Renderer& renderer;
	GFFN_Camera camera;
	GameWorldObjects game_world_objects;

	GFFN_GameWorld(GFFN_Renderer &renderer) : renderer(renderer), camera(GFFN_Camera(renderer.get_sdl_renderer())) {}
	~GFFN_GameWorld() {}

	long unsigned int add_object(std::unique_ptr<GFFN_GameObject> &object) {
		if (object->get_object_type() == GFFN_OBJECT_TYPE_CHARACTER) {
			num_characters++;
		}
		else if (object->get_object_type() == GFFN_OBJECT_TYPE_NPC) {
			num_npcs++;
		}
		else if (object->get_object_type() == GFFN_OBJECT_TYPE_UI_OBJECT) {
			num_ui_objects++;
		}
		else if (object->get_object_type() == GFFN_OBJECT_TYPE_DISMEMBERED_BODY_PART) {
			num_dismembered_body_parts++;
		}
		else if (object->get_object_type() == GFFN_OBJECT_TYPE_ENVIRONMENTAL_OBJECT) {
			num_environmental_objects++;
		}
		else if (object->get_object_type() == GFFN_OBJECT_TYPE_STRAIGHT_PROJECTILE) {
			num_straight_projectiles++;
		}
		else {
			throw GFFN_Exception(std::string("Unknown object type passed into add_object for GFFN_GameWorld"));
		}

		long unsigned int object_id = object->get_object_id();
		game_world_objects.add_object(std::move(object));
		return object_id;
	}

	void remove_object(long unsigned int object_id) {
		if (!game_world_objects.object_exists(object_id)) {
			return;
		}
		GFFN_GameObject* const object = game_world_objects.get_object(object_id);
		if (object->get_object_id() == object_id) {
			GFFN_ObjectType object_type = object->get_object_type();
			if (object_type == GFFN_OBJECT_TYPE_CHARACTER) {
				num_characters--;
			}
			else if (object_type == GFFN_OBJECT_TYPE_NPC) {
				num_npcs--;
			}
			else if (object_type == GFFN_OBJECT_TYPE_UI_OBJECT) {
				num_ui_objects--;
			}
			else if (object_type == GFFN_OBJECT_TYPE_DISMEMBERED_BODY_PART) {
				num_dismembered_body_parts--;
			}
			else if (object_type == GFFN_OBJECT_TYPE_ENVIRONMENTAL_OBJECT) {
				num_environmental_objects--;
			}
			else if (object_type == GFFN_OBJECT_TYPE_STRAIGHT_PROJECTILE) {
				num_straight_projectiles--;
			}
			else {
				throw GFFN_Exception(std::string("Unknown object type passed into add_object for GFFN_GameWorld"));
			}
		}
		game_world_objects.remove_object(object_id);
	}

	WorldCoordinate get_mouse_position_as_coordinate(GFFN_Renderer& renderer) {
		return renderer.get_mouse_position_as_coordinate(camera);
	}

	template <class T>
	void dismember_character(T* const object, physics::NormalizedVector3D throw_vector, double throw_velocity_magnitude) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_real_distribution<> dist(0.3, 1);
		for (int i = 0; i < 6; i++) {
			std::pair<SDL_Texture*, SDL_Rect*> part_image_info = renderer.character_dismemberment_images->get_image(i);
			SDL_Texture* part_texture = part_image_info.first;
			SDL_Rect* part_source_rect = part_image_info.second;
			std::string texture_filename = std::string("C:\\Users\\guzzo\\Documents\\workspaces\\unnamed_game\\unnamed_game\\textures\\small_shadow.png");
			std::unique_ptr<GFFN_DismemberedBodyPart> part =
				std::make_unique<GFFN_DismemberedBodyPart>(object->get_floor_coords(), part_texture, renderer.get_texture(texture_filename), part_source_rect);
			gffn::physics::NormalizedVector3D part_throw_vector = throw_vector;
			double rotation = (dist(gen) * 90) - 45;
			part_throw_vector.rotate_xy(rotation);
			physics::ObjectPhysicsController &part_physics_controller = part->get_physics_controller();
			part_physics_controller.set_height(30);
			part_physics_controller.set_ground_coef_friction(20);
			
			physics::Vector3D part_velocity = physics::Vector3D(part_throw_vector.x * throw_velocity_magnitude * dist(gen), part_throw_vector.y * throw_velocity_magnitude * dist(gen), 0);
			//part_velocity.z = throw_velocity_magnitude * 0.2;
			part_physics_controller.set_velocity(part_velocity);
			
			std::unique_ptr<gffn::GFFN_GameObject> part_ptr = std::move(part);
			add_object(part_ptr);
		}
	}

	void event_handler() {
		while (!events::event_queue.empty()) {
			if (std::holds_alternative<events::ProjectileHitEvent>(events::event_queue.front())) {
				events::ProjectileHitEvent projectile_hit_event = std::get<events::ProjectileHitEvent>(events::event_queue.front());
				events::event_queue.pop();

				GFFN_GridObject *object = projectile_hit_event.object;
				GFFN_StraightProjectile* projectile = projectile_hit_event.projectile;
				if (object == nullptr || projectile == nullptr) {
					continue;
				}

				GFFN_ObjectType object_type = object->get_object_type();
				long unsigned int object_id = object->get_object_id();
				GFFN_ObjectType projectile_type = projectile->get_object_type();
				long unsigned int projectile_id = projectile->get_object_id();

				if (object_type == GFFN_OBJECT_TYPE_CHARACTER && projectile_type == GFFN_OBJECT_TYPE_STRAIGHT_PROJECTILE) {
					GFFN_Character* character = static_cast<GFFN_Character*>(object);
					if (character->is_dead()) {
						dismember_character<GFFN_Character>(character, projectile->get_velocity_direction(), 700);
						remove_object(object_id);
					}
				}
				else if (object_type == GFFN_OBJECT_TYPE_NPC && projectile_type == GFFN_OBJECT_TYPE_STRAIGHT_PROJECTILE) {
					GFFN_NPC* npc = static_cast<GFFN_NPC*>(object);
					if (npc->is_dead()) {
						dismember_character<GFFN_NPC>(npc, projectile->get_velocity_direction(), 700);
						remove_object(object_id);
					}
				}
				else if (object_type == GFFN_OBJECT_TYPE_ENVIRONMENTAL_OBJECT) {
					// TODO: Handle projectile hitting environmental object??
				}
				else {
					//throw GFFN_Exception("Unknown object type in event handler.");
				}
			}
			else if (std::holds_alternative<events::ExplosionEvent>(events::event_queue.front())) {
				// TODO: Handle explosion event.
			}
		}
	}

	void tick(GFFN_Renderer &renderer, double delta_time_seconds) {
		try {
			event_handler();
		}
		catch (std::exception& e) {
			printf("Exception caught in event_handler %s\n", e.what());
			throw;
		}

		camera.tick(delta_time_seconds);

		//game_world_objects.clear_objects_by_y();

		for (auto it = game_world_objects.begin(); it != game_world_objects.end();) {
			GFFN_GameObject* const object = *it;

			if (object->to_remove()) {
				if (object->get_object_type() == GFFN_OBJECT_TYPE_CHARACTER) {
					num_characters--;
					printf("character removed. %d left.\n", num_npcs);
				}
				else if (object->get_object_type() == GFFN_OBJECT_TYPE_NPC) {
					num_npcs--;
					printf("NPC removed. %d left.\n", num_npcs);
				}
				else if (object->get_object_type() == GFFN_OBJECT_TYPE_UI_OBJECT) {
					num_ui_objects--;
				}
				else if (object->get_object_type() == GFFN_OBJECT_TYPE_DISMEMBERED_BODY_PART) {
					num_dismembered_body_parts--;
				}
				else if (object->get_object_type() == GFFN_OBJECT_TYPE_ENVIRONMENTAL_OBJECT) {
					num_environmental_objects--;
				}
				else if (object->get_object_type() == GFFN_OBJECT_TYPE_STRAIGHT_PROJECTILE) {
					num_straight_projectiles--;
				}
				else {
					throw GFFN_Exception(std::string("Unknown object type passed into add_object for GFFN_GameWorld"));
				}
				it = game_world_objects.remove_object(it);
				continue;
			}

			switch (object->get_object_type()) {
			case GFFN_OBJECT_TYPE_CHARACTER: {
				GFFN_Character* const character = static_cast<GFFN_Character*>(object);
				try{
					character->tick(delta_time_seconds);
				}
				catch (std::exception& e) {
					printf("Exception caught in character->tick %s\n", e.what());
					throw;
				}
				break;
			}
			case GFFN_OBJECT_TYPE_NPC: {
				GFFN_NPC* const npc = static_cast<GFFN_NPC*>(object);
				npc->tick(delta_time_seconds);
				break;
			}
			case GFFN_OBJECT_TYPE_DISMEMBERED_BODY_PART: {
				GFFN_DismemberedBodyPart* const part = static_cast<GFFN_DismemberedBodyPart*>(object);
				if (part->get_physics_controller().get_velocity().is_zero()) {
					renderer.bake_object_onto_floor<GFFN_DismemberedBodyPart>(part);
					it = game_world_objects.remove_object(it);
					num_dismembered_body_parts--;
					continue;
				}
				part->tick(delta_time_seconds);
				break; 
			}
			case GFFN_OBJECT_TYPE_STRAIGHT_PROJECTILE: {
				GFFN_StraightProjectile* const projectile = static_cast<GFFN_StraightProjectile*>(object);
				/*if (projectile->to_remove()) {
					it = game_world_objects.erase(it);
					num_straight_projectiles--;
					continue;
				}*/
				projectile->tick(delta_time_seconds);
				break;
			}
			case GFFN_OBJECT_TYPE_ENVIRONMENTAL_OBJECT: {
				// nothing yet
				break;
			}
			default:
				printf("Unknown object type in game world.\n");
			}

			// Add object to objects_by_y if it is visible:

			/*if (camera.object_in_viewport(object->get_render_rect(), object->get_height_offset())) {
				game_world_objects.add_to_objects_by_y(object);
			}*/
			++it;
		}

		// Render
		//game_world_objects.sort_objects_by_y();

		try {
			renderer.render_everything_in_viewport(game_world_objects.get_objects_by_y(), camera);
		}
		catch (std::exception& e) {
			printf("Exception caught in render_everything_in_viewport %s\n", e.what());
			throw;
		}
	}
};

} // end namespace gffn