#pragma once

#include <vector>
#include <format>
#include <chrono>

#include <SDL.h>
#include <SDL_image.h>

#include <gffn_exception.h>
#include <gffn_utils.h>

namespace gffn {

typedef enum : int {
	GFFN_ANIMATION_WALK_UP = 0,
	GFFN_ANIMATION_WALK_TOP_LEFT,
	GFFN_ANIMATION_WALK_LEFT,
	GFFN_ANIMATION_WALK_BOTTOM_LEFT,
	GFFN_ANIMATION_WALK_DOWN,
	GFFN_ANIMATION_WALK_BOTTOM_RIGHT,
	GFFN_ANIMATION_WALK_RIGHT,
	GFFN_ANIMATION_WALK_TOP_RIGHT,
	GFFN_ANIMATION_IDLE_UP,
	GFFN_ANIMATION_IDLE_TOP_LEFT,
	GFFN_ANIMATION_IDLE_LEFT,
	GFFN_ANIMATION_IDLE_BOTTOM_LEFT,
	GFFN_ANIMATION_IDLE_DOWN,
	GFFN_ANIMATION_IDLE_BOTTOM_RIGHT,
	GFFN_ANIMATION_IDLE_RIGHT,
	GFFN_ANIMATION_IDLE_TOP_RIGHT,
	GFFN_ANIMATION_THROWN,
	GFFN_CHARACTER_ANIMATION_STATE_END
} GFFN_CharacterAnimationState;

class GFFN_Animations {
	int frame_width;
	int frame_height;
	SDL_Texture* animation_texture; // This texture contains all of the frames of the animation.
	std::shared_ptr<SDL_Rect> frame_rect; // This is the rectangle of the current frame.
	std::pair<int, int> current_frame; // The first value is the state, the second value is the frame number of that state.
	int fps;
	int number_of_states;
	int number_of_frames_per_state;
public:
	GFFN_Animations(SDL_Texture* animation_texture, int fps, int frame_width, int frame_height) : 
	animation_texture(animation_texture), fps(fps), current_frame(GFFN_ANIMATION_IDLE_BOTTOM_LEFT, 0), frame_width(frame_width), frame_height(frame_height) {
		int width, height;
		SDL_QueryTexture(animation_texture, NULL, NULL, &width, &height);
		if (width % frame_width != 0) {
			throw GFFN_Exception(std::format("Animation texture width is not a multiple of {}.", frame_width));
		}
		if (height % frame_height != 0) {
			throw GFFN_Exception(std::format("Animation texture height is not a multiple of {}.", frame_height));
		}
		number_of_states = height / frame_height;
		number_of_frames_per_state = width / frame_width;
		frame_rect = std::make_shared<SDL_Rect>();
	}
	~GFFN_Animations() {}

	// Returns information to get the current frame. 
	// The first value contains the entire animation texture, the second value contains the rectangle of the current frame.
	std::pair<SDL_Texture*, SDL_Rect*> get_current_frame() {
		int current_state = current_frame.first;
		int current_frame_number = current_frame.second;
		*frame_rect = { current_frame_number * frame_width, current_state * frame_height, frame_width, frame_height };
		return std::make_pair(animation_texture, frame_rect.get());
	}

	void set_fps(int fps) {
		this->fps = fps;
	}

	void set_state(int state) {
		current_frame.first = state;
	}	

	void next_frame() {
		current_frame.second = (current_frame.second + 1) % number_of_frames_per_state;
	}

	std::chrono::high_resolution_clock::time_point last_time = std::chrono::high_resolution_clock::now();

	void tick() {
		auto current_time = std::chrono::high_resolution_clock::now();
		auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time);
		if (time_diff.count() > 1000 / fps) {
			next_frame();
			last_time = current_time;
		}
	}
};



}