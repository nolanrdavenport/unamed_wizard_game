#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <variant>

#include <SDL.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <SDL_rect.h>

#include <gffn_window.h>
#include <gffn_exception.h>
#include <gffn_game_object.h>
#include <gffn_camera.h>
#include <gffn_utils.h>
#include <gffn_game_world_objects.h>
class GFFN_GameObject;

namespace gffn {

class GFFN_Renderer {
	GFFN_Window window;
	SDL_Renderer* renderer;
	const int renderer_logical_width = RENDERER_LOGICAL_WIDTH;
	const int renderer_logical_height = RENDERER_LOGICAL_HEIGHT;
	std::unordered_map<std::string, SDL_Texture*> textures; // This maps filenames to textures, and the filenames will be used to retrieve the textures.
public:
	SDL_Texture* get_texture(std::string const &filename) { 
		if(textures.count(filename) == 0) {
			textures[filename] = IMG_LoadTexture(renderer, filename.c_str());
		}
		return textures[filename]; 
	}

	std::shared_ptr<GFFN_GroundObject> ground_object; // Look into if this is needed

	std::unique_ptr<GFFN_MultiImage> character_dismemberment_images;

	GFFN_Renderer(std::string const &game_name);
	~GFFN_Renderer();
	SDL_Renderer* get_sdl_renderer() { return renderer; }
	template <class T> void render_character_objects(std::shared_ptr<T> character, GFFN_Camera camera);
	//template <class T> void render_object_relative_to_camera(std::shared_ptr<T> object, GFFN_Camera camera, bool render_shadow = true);
	void render_object_relative_to_camera(GFFN_GameObject* const object, GFFN_Camera camera);
	void render_everything_in_viewport(objects_by_y_t& game_world_objects, GFFN_Camera camera);
	WorldCoordinate get_mouse_position_as_coordinate(GFFN_Camera camera);
	int get_renderer_width() {
		int width;
		SDL_RenderGetLogicalSize(renderer, &width, nullptr);
		return width;
	}
	int get_renderer_height() {
		int height;
		SDL_RenderGetLogicalSize(renderer, nullptr, &height);
		return height;
	}

	template <class T>
	void bake_object_onto_floor(T* const object) {
		SDL_Rect object_rect = *object->get_render_rect();
		SDL_Rect ground_object_rect = *ground_object->get_render_rect();
		SDL_Rect object_rect_relative_to_ground = SDL_Rect(
			object_rect.x - ground_object_rect.x,
			object_rect.y - ground_object_rect.y,
			object_rect.w,
			object_rect.h
		);


		SDL_SetRenderTarget(renderer, ground_object->get_texture());

		SDL_RenderCopy(renderer, object->get_texture(), object->get_source_rect(), &object_rect_relative_to_ground);
	}
};

} // end namespace gffn