#pragma once

#include <SDL.h>
#include <SDL_render.h>

#include <gffn_utils.h>
#include <gffn_physics.h>

namespace gffn{
class GFFN_Camera {
	// This is used so that it's easy to set the center pos of the camera to the center of a character, or wherever else
	physics::ObjectPhysicsController camera_physics_controller;
	WorldCoordinate camera_center_pos;
	SDL_Texture* camera_texture;
	SDL_Renderer* renderer;
public:
	static constexpr int WIDTH_OF_VIEWPORT_AT_ZOOM_1 = RENDERER_LOGICAL_WIDTH;
	static constexpr int HEIGHT_OF_VIEWPORT_AT_ZOOM_1 = RENDERER_LOGICAL_HEIGHT;
	// in logical pixels. This is the length and width of the viewport in pixels, with each unit of GFFN_WorldCoordinate being one pixel.
	SDL_Rect viewport;
	GFFN_Camera(SDL_Renderer* renderer) : 
	renderer(renderer), camera_center_pos(WorldCoordinate(WORLD_GRID_WIDTH*50, WORLD_GRID_HEIGHT*50, 0)), 
	viewport(SDL_Rect(0, 0, WIDTH_OF_VIEWPORT_AT_ZOOM_1, HEIGHT_OF_VIEWPORT_AT_ZOOM_1)),
	camera_physics_controller(WorldCoordinate(WORLD_GRID_WIDTH * 50, WORLD_GRID_HEIGHT * 50, 0), 15) {
		camera_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WIDTH_OF_VIEWPORT_AT_ZOOM_1, HEIGHT_OF_VIEWPORT_AT_ZOOM_1);
	} // for starting a game at the origin
	~GFFN_Camera() {}

	void set_camera_center_pos(WorldCoordinate new_pos) {
		camera_center_pos = new_pos;
		viewport.x = (int)(camera_center_pos.x - ((double)viewport.w / 2));
		viewport.y = (int)(camera_center_pos.y - ((double)viewport.h / 2));
	}
	void move_to_position(WorldCoordinate new_pos) {
		physics::Vector3D cam_to_pos = physics::Vector3D(camera_center_pos, new_pos);
		if (cam_to_pos.magnitude() < 5) {
			//set_camera_center_pos(character->get_center_coords());
			return;
		}
		physics::NormalizedVector3D cam_move_force_normalized = physics::NormalizedVector3D(cam_to_pos);
		physics::Vector3D camera_move_force = cam_move_force_normalized * cam_to_pos.magnitude() * 7000;
		camera_physics_controller.add_force(camera_move_force);
	}

	void tick(double delta_time_seconds) {
		camera_physics_controller.tick(delta_time_seconds);
		set_camera_center_pos(camera_physics_controller.get_floor_coords());
		if (viewport.x < 150) {
			viewport.x = 160;
		}
		if (viewport.y < 150) {
			viewport.y = 160;
		}
		if (viewport.x + viewport.w > WORLD_GRID_WIDTH * 99 - 50) {
			viewport.x = WORLD_GRID_WIDTH * 99 - viewport.w - 60;
		}
		if (viewport.y + viewport.h > WORLD_GRID_HEIGHT * 99 - 50) {
			viewport.y = WORLD_GRID_HEIGHT * 99 - viewport.h - 60;
		}
	}

	bool object_in_viewport(SDL_Rect *object_rect, int vertical_offset) const {
		if (object_rect->y + object_rect->h < viewport.y) {
			return false;
		}
		if (object_rect->y - vertical_offset > viewport.y + viewport.h) {
			return false;
		}
		if (object_rect->x + object_rect->w < viewport.x) {
			return false;
		}
		if (object_rect->x > viewport.x + viewport.w) {
			return false;
		}
		return true;
	}

	SDL_Texture* get_camera_texture() { return camera_texture; }

	void zoom(double zoom_factor) {
		viewport.w = (int)(WIDTH_OF_VIEWPORT_AT_ZOOM_1 * zoom_factor);
		viewport.h = (int)(HEIGHT_OF_VIEWPORT_AT_ZOOM_1 * zoom_factor);
		SDL_DestroyTexture(camera_texture);
		camera_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, viewport.w, viewport.h);
	}

	// TODO: Add velocity if desired, not needed now.
};
} // end namespace gffn