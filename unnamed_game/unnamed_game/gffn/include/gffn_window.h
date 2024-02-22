#pragma once

#include <SDL.h>
#include <SDL_video.h>

#include <iostream>
#include <string>

namespace gffn {
class GFFN_Window {
	std::string name;
	SDL_Window* sdl_window;
public:
	GFFN_Window(const std::string &name);
	~GFFN_Window();
	SDL_Window* get_sdl_window() { return sdl_window; }
};

} // end namespace gffn