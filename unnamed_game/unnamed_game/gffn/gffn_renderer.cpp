#include <gffn_renderer.h>

namespace gffn {
    GFFN_Renderer::GFFN_Renderer(std::string const& game_name) : window{ game_name } {
        // Renderer setup
        //renderer = SDL_CreateRenderer(window.get_sdl_window(), -1, SDL_RENDERER_ACCELERATED);
        renderer = SDL_CreateRenderer(window.get_sdl_window(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer == nullptr) {
            throw GFFN_Exception(std::string("Failure to create SDL renderer"));
        }
        SDL_RenderSetLogicalSize(renderer, renderer_logical_width, renderer_logical_height);

        SDL_RendererInfo renderer_info;
        SDL_GetRendererInfo(renderer, &renderer_info);
        std::cout << "Renderer name: " << renderer_info.name << std::endl;

        character_dismemberment_images = std::make_unique<GFFN_MultiImage>(get_texture("textures/goblin/goblin_1_limbs.png"));

        // TODO: Multi layer? Having a camera texture instead of having to do math to know what gets rendered. Might be more performant.
        SDL_Texture* ground_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, WORLD_GRID_WIDTH * 100, WORLD_GRID_HEIGHT * 100);
        if (ground_texture == nullptr) {
			throw GFFN_Exception(std::string("Failure to create ground texture"));
		}
        SDL_SetRenderTarget(renderer, ground_texture);
        for (SDL_Rect rect(0, 0, 100, 100); rect.x < WORLD_GRID_WIDTH * 100; rect.x += 100) {
            for (rect.y = 0; rect.y < WORLD_GRID_HEIGHT * 100; rect.y += 100) {
                if (SDL_RenderCopy(renderer, 
                    get_texture(std::string("C:\\Users\\guzzo\\Documents\\workspaces\\unnamed_game\\unnamed_game\\textures\\ground_yellow_flowers.png")), 
                    nullptr, &rect) < 0) {
					throw GFFN_Exception(std::string("Failure to copy texture to ground texture"));
				}
			}
        }
        ground_object = std::make_shared<GFFN_GroundObject>(ground_texture);
    }
    GFFN_Renderer::~GFFN_Renderer() {
        /*for (auto const& texture : still_textures) {
            SDL_DestroyTexture(texture.second);
        }
        for (auto const& texture : animation_textures) {
            SDL_DestroyTexture(texture.second);
        }
        SDL_DestroyTexture(ground_object->get_texture());*/
        for(auto const& [key, value] : textures) {
			SDL_DestroyTexture(value);
		}
        SDL_DestroyRenderer(renderer);
    }

    void GFFN_Renderer::render_object_relative_to_camera(GFFN_GameObject* const object, GFFN_Camera camera) {
        if (object->get_hidden()) {
            return;
        }
        SDL_Rect object_rect_relative_to_camera{};
        SDL_Rect* object_render_rect = object->get_render_rect();
        object_rect_relative_to_camera.x = object_render_rect->x - camera.viewport.x;
        object_rect_relative_to_camera.y = object_render_rect->y - camera.viewport.y;
        object_rect_relative_to_camera.w = object_render_rect->w;
        object_rect_relative_to_camera.h = object_render_rect->h;
        if (object->get_shadow_texture() != nullptr) {
            // Create shadow rect in relation to character's rect.
            SDL_Rect shadow_rect = *object->get_shadow_render_rect();
            SDL_Rect shadow_rect_relative_to_camera{};
            shadow_rect_relative_to_camera.x = shadow_rect.x - camera.viewport.x;
            shadow_rect_relative_to_camera.y = shadow_rect.y - camera.viewport.y;
            shadow_rect_relative_to_camera.w = shadow_rect.w;
            shadow_rect_relative_to_camera.h = shadow_rect.h;
            SDL_RenderCopy(renderer, object->get_shadow_texture(), nullptr, &shadow_rect_relative_to_camera);
        }
        object_rect_relative_to_camera.y -= static_cast<int>(object->get_height_offset());
        SDL_RenderCopy(renderer, object->get_texture(), object->get_source_rect(), &object_rect_relative_to_camera);
    }

    void GFFN_Renderer::render_everything_in_viewport(objects_by_y_t& game_world_objects, GFFN_Camera camera) {
        static std::vector<GFFN_GameObject*> sorted_row_of_objects;

        SDL_SetRenderTarget(renderer, camera.get_camera_texture());

        // Render the ground first, as it is below everything.
        SDL_Rect *ground_render_rect = ground_object->get_render_rect();
        SDL_Rect ground_rect_relative_to_camera{};
        ground_rect_relative_to_camera.x = ground_render_rect->x - camera.viewport.x;
        ground_rect_relative_to_camera.y = ground_render_rect->y - camera.viewport.y;
        ground_rect_relative_to_camera.w = ground_render_rect->w;
        ground_rect_relative_to_camera.h = ground_render_rect->h;
        SDL_RenderCopy(renderer, ground_object->get_texture(), ground_object->get_source_rect(), &ground_rect_relative_to_camera);

        int top_left_grid_x = (camera.viewport.x / 100) - 1;
        int top_left_grid_y = (camera.viewport.y / 100) - 1;
        int bottom_right_grid_x = ((camera.viewport.x + camera.viewport.w) / 100) + 1;
        int bottom_right_grid_y = ((camera.viewport.y + camera.viewport.h) / 100) + 6;
        for (int j = top_left_grid_y; j <= bottom_right_grid_y; ++j) {
            for(int i = top_left_grid_x; i <= bottom_right_grid_x; ++i) {
                if (i < 0 || i >= WORLD_GRID_WIDTH || j < 0 || j >= WORLD_GRID_HEIGHT) {
					continue;
				}
                /*std::sort(world_grid[i][j].begin(), world_grid[i][j].end(), [this](GFFN_GameObject* first, GFFN_GameObject* second){
                    return first->get_y() < second->get_y();
                });*/
				for(GFFN_GameObject* const object : world_grid[i][j]) {
                    if (object->get_hidden()) continue;
                    if (object->get_render_rect()->y > camera.viewport.y + camera.viewport.h) continue;
					//render_object_relative_to_camera(object, camera);
                    sorted_row_of_objects.push_back(object);
				}
			}
            std::sort(sorted_row_of_objects.begin(), sorted_row_of_objects.end(), [this](GFFN_GameObject* first, GFFN_GameObject* second){
				return first->get_y() < second->get_y();
			});
            for(GFFN_GameObject* const object : sorted_row_of_objects) {
				render_object_relative_to_camera(object, camera);
			}
			sorted_row_of_objects.clear();
		}

        SDL_SetRenderTarget(renderer, nullptr);
        SDL_RenderCopy(renderer, camera.get_camera_texture(), nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    WorldCoordinate GFFN_Renderer::get_mouse_position_as_coordinate(GFFN_Camera camera) {
        SDL_Rect camera_viewport = camera.viewport;
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        float logical_mouse_x, logical_mouse_y;
        SDL_RenderWindowToLogical(renderer, mouse_x, mouse_y, &logical_mouse_x, &logical_mouse_y);

        float viewport_mouse_x = (logical_mouse_x/RENDERER_LOGICAL_WIDTH) * camera_viewport.w;
        float viewport_mouse_y = (logical_mouse_y/RENDERER_LOGICAL_HEIGHT) * camera_viewport.h;

        int mouse_x_coord = camera.viewport.x + (int)viewport_mouse_x;
        int mouse_y_coord = camera.viewport.y + (int)viewport_mouse_y;

        WorldCoordinate coordinate;
        coordinate.x = (double)mouse_x_coord;
        coordinate.y = (double)mouse_y_coord;
        return coordinate;
    }

}  // end namespace gffn