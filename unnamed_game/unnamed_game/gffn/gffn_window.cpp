#include <gffn_window.h>
#include <gffn_exception.h>

namespace gffn {

	GFFN_Window::GFFN_Window(const std::string& name) {
		this->name = name;

		SDL_DisplayMode dm;
		SDL_GetDesktopDisplayMode(0, &dm);
		int monitor_width = dm.w;
		int monitor_height = dm.h;

		sdl_window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, monitor_width, monitor_height, SDL_WINDOW_BORDERLESS);
		if (sdl_window == nullptr) {
			throw GFFN_Exception(std::string("Failure to create STL window"));
		}
	}

	GFFN_Window::~GFFN_Window() {
		SDL_DestroyWindow(sdl_window);
	}

} // end namespace gffn