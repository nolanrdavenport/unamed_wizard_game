// sdl_game_1.cpp : Defines the entry point for the application.
//

#include "unnamed_game.h"

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_image.h>
#include <SDL_keyboard.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <gffn_events.h>
#include <gffn_game_object.h>
#include <gffn_game_world.h>
#include <gffn_renderer.h>
#include <gffn_utils.h>
#include <gffn_window.h>
#include <gffn_physics.h>

#include <chrono>
#include <csignal>
#include <format>
#include <iostream>
#include <memory>
#include <thread>
#include <random>

int main(int argc, char* argv[]) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> world_coord_rand(100, gffn::WORLD_GRID_WIDTH*99);

    /*try {*/
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
            std::cout << "Error SDL2 Initialization : " << SDL_GetError() << std::endl;
            return EXIT_FAILURE;
        }

        /*if (SDL_ShowCursor(SDL_DISABLE) != SDL_DISABLE) {
            std::cout << "Error disabling cursor" << std::endl;
        }*/

        SDL_Surface *surface = IMG_Load("textures/player_pointer.png");
        SDL_Cursor *cursor = SDL_CreateColorCursor(surface, 25, 25);
        SDL_SetCursor(cursor);

        gffn::GFFN_Renderer renderer(std::string("unamed_game"));  // temp name

        gffn::GFFN_GameWorld game_world(renderer);

        game_world.camera.zoom(1.5);

        SDL_Texture* texture = IMG_LoadTexture(renderer.get_sdl_renderer(), "textures/alexs_pine_tree.png");

        for (int i = 0; i < 1000; i++) {
            double x = world_coord_rand(gen);
            double y = world_coord_rand(gen);
            
            std::unique_ptr<gffn::GFFN_GameObject> env_obj = 
                std::make_unique<gffn::GFFN_EnvironmentalObject>(gffn::WorldCoordinate(x, y, 0), texture, renderer.get_texture("textures/small_shadow.png"));

            game_world.add_object(env_obj);
        }

        gffn::GFFN_Character* player_character = nullptr;

        {
            std::unique_ptr<gffn::GFFN_GameObject> object = std::make_unique<gffn::GFFN_Character>(
                gffn::GFFN_ObjectType::GFFN_OBJECT_TYPE_CHARACTER,
                renderer.get_texture("textures/wizard/generic.png"),
                renderer.get_texture("textures/character_shadow.png"),
                5,
                gffn::WorldCoordinate(gffn::WORLD_GRID_WIDTH * 50, gffn::WORLD_GRID_HEIGHT * 50, 0)
            );
            player_character = static_cast<gffn::GFFN_Character*>(object.get());
            long unsigned int player_character_id = game_world.add_object(object);
        }

        for (int i = 0; i < 1000; i++) {
            double x = world_coord_rand(gen);
            double y = world_coord_rand(gen);

            gffn::NPC_info npc_info;
            npc_info.floor_coords = gffn::WorldCoordinate(x, y, 0);
            npc_info.animation_texture = renderer.get_texture("textures/goblin/goblin_1.png");
            npc_info.shadow_texture = renderer.get_texture("textures/character_shadow.png");
            //npc_info.hp = 5; TODO add this
            npc_info.animation_fps = 5;
            npc_info.character_type = gffn::GFFN_CharacterType::GFFN_GOBLIN_1;

            std::unique_ptr<gffn::GFFN_GameObject> npc = std::make_unique<gffn::GFFN_NPC>(npc_info);
            game_world.add_object(npc);
        }

        bool close_window = false;
        auto previous_time = std::chrono::high_resolution_clock::now();
        auto second_timer_start = std::chrono::high_resolution_clock::now();
        int frames = 0;
        while (close_window == false) {
            frames++;
            auto delta_time = std::chrono::high_resolution_clock::now() - previous_time;
            auto second_timer_end = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::duration<double>>(second_timer_end - second_timer_start).count() > 1.0) {
                std::cout << "----------- debug info -------------" << std::endl;
                std::cout << "FPS: " << frames << std::endl;
                std::cout << "Num characters: " << game_world.num_characters << std::endl;
                std::cout << "Num NPCs: " << game_world.num_npcs << std::endl;
                std::cout << "Num straight projectiles: " << game_world.num_straight_projectiles << std::endl;
                std::cout << "Num dismembered body parts: " << game_world.num_dismembered_body_parts << std::endl;
                std::cout << "Num environmental objects: " << game_world.num_environmental_objects << std::endl;
                frames = 0;
                second_timer_start = std::chrono::high_resolution_clock::now();
            }
            previous_time = std::chrono::high_resolution_clock::now();
            double delta_time_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(delta_time).count();
            gffn::WorldCoordinate mouse_position_coord = game_world.get_mouse_position_as_coordinate(renderer);
            gffn::WorldCoordinate player_pointer_coord = mouse_position_coord;

            SDL_Event e;
            if (SDL_PollEvent(&e) > 0) {
                if (e.type == SDL_QUIT) {
                    close_window = true;
                }
                if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                        close_window = true;
                    }
                    if (e.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                        //player_character->throw_to(mouse_position_coord, 500.0);
                        //player_character->add_force(gffn::physics::Vector3D(0, 0, 20000.0));
                    }
                    //if (e.key.keysym.scancode == SDL_SCANCODE_F) {  // Place character :)
                    //    std::shared_ptr<gffn::GFFN_NPC> npc = std::make_shared<gffn::GFFN_NPC>(
                    //        gffn::GFFN_ObjectType::GFFN_OBJECT_TYPE_NPC,
                    //        renderer.get_sdl_renderer(),
                    //        renderer.animation_textures.at(gffn::GFFN_CharacterType::GFFN_GOBLIN_1),
                    //        renderer.still_textures.at(gffn::GFFN_TextureEnum::GFFN_TEXTURE_CHARACTER_SHADOW),
                    //        5,
                    //        mouse_position_coord
                    //    );
                    //    game_world.add_object<gffn::GFFN_NPC>(npc);
                    //}
                    if (e.key.keysym.scancode == SDL_SCANCODE_R) {
                        player_character->set_floor_coords(gffn::WorldCoordinate(gffn::WORLD_GRID_WIDTH*50,gffn::WORLD_GRID_HEIGHT*50, 0));
                        player_character->change_hp(100);
					}
                }
                if (e.type == SDL_MOUSEBUTTONDOWN) {
                    /*if (e.button.button == SDL_BUTTON_LEFT) {
                        if (!player_character->is_being_thrown()) {
                            std::shared_ptr<gffn::GFFN_StraightProjectile> projectile = std::make_shared<gffn::GFFN_StraightProjectile>(
                                player_character->get_floor_coords(),
                                mouse_position_coord,
                                1500.0,
                                2.0,
                                renderer.still_textures.at(gffn::GFFN_TextureEnum::GFFN_TEXTURE_THROWABLE_EXPLOSIVE),
                                renderer.still_textures.at(gffn::GFFN_TextureEnum::GFFN_TEXTURE_SMALL_SHADOW)
                            );
                            game_world.add_object<gffn::GFFN_StraightProjectile>(projectile);
                        }
                    }*/
                }
                if (e.type == SDL_MOUSEWHEEL) {
                    static double zoom_factor = 1.5;
                	if (e.wheel.y > 0) {
                        zoom_factor -= 0.1;
					}
					else if (e.wheel.y < 0) {
                        zoom_factor += 0.1;
					}

                    if (zoom_factor > 5.0) {
                        zoom_factor = 5.0;
					}
                    else if (zoom_factor < 0.2) {
						zoom_factor = 0.2;
                    }
                    game_world.camera.zoom(zoom_factor);
                }
            }



            const Uint8* keystate = SDL_GetKeyboardState(nullptr);

            // Player movement
            gffn::physics::Vector3D player_move_vector; // will be normalized later
            bool player_move_key_pressed = false;
            if (keystate[SDL_SCANCODE_A]) {
                player_move_vector.x -= 1;
                player_move_key_pressed = true;
                //player_character->add_force(gffn::physics::Vector3D(-100.0, 0, 0));
            }
            if (keystate[SDL_SCANCODE_D]) {
                player_move_vector.x += 1;
                player_move_key_pressed = true;
                //player_character->add_force(gffn::physics::Vector3D(100.0, 0, 0));
            }
            if (keystate[SDL_SCANCODE_W]) {
                player_move_vector.y -= 1;
                player_move_key_pressed = true;
                //player_character->add_force(gffn::physics::Vector3D(0, -100.0, 0));
            }
            if (keystate[SDL_SCANCODE_S]) {
                player_move_vector.y += 1;
                player_move_key_pressed = true;
                //player_character->add_force(gffn::physics::Vector3D(0, 100.0, 0));
            }
            if (keystate[SDL_SCANCODE_SPACE]) {
                player_character->add_force(gffn::physics::Vector3D(0, 0, 500000.0));
            }
            if (keystate[SDL_SCANCODE_F]) {
                gffn::NPC_info npc_info;
                npc_info.floor_coords = mouse_position_coord;
                npc_info.animation_texture = renderer.get_texture("textures/goblin/goblin_1.png");
                npc_info.shadow_texture = renderer.get_texture("textures/character_shadow.png");
                //npc_info.hp = 5; TODO add this
                npc_info.animation_fps = 5;
                npc_info.character_type = gffn::GFFN_CharacterType::GFFN_GOBLIN_1;

                std::unique_ptr<gffn::GFFN_GameObject> npc = std::make_unique<gffn::GFFN_NPC>(npc_info);
                game_world.add_object(npc);
			}

            if (!player_move_vector.is_zero()) {
                gffn::physics::NormalizedVector3D player_move_vector_normalized(player_move_vector);
                gffn::physics::Vector3D player_move_force(player_move_vector_normalized.x*300000.0, player_move_vector_normalized.y* 300000.0, 0);
                player_character->add_force(player_move_force);
            }
            // Handle camera positioning
            game_world.camera.move_to_position(player_character->get_center_coords());
            const Uint32 mouse_state = SDL_GetMouseState(nullptr, nullptr);
            if (mouse_state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
				gffn::WorldCoordinate mouse_coordinates = game_world.get_mouse_position_as_coordinate(renderer);
                gffn::WorldCoordinate player_coordinates = player_character->get_floor_coords();
                gffn::WorldCoordinate center_coordinates = player_coordinates.get_coordinate_between_percent_from_source(mouse_coordinates, 99);
                game_world.camera.move_to_position(center_coordinates);
			}
            if (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                static std::uniform_real_distribution<> shooting_random(-14.0, 14.0);
                static double time_since_last_throw = 100.0;
                time_since_last_throw += delta_time_seconds;
                gffn::physics::NormalizedVector3D player_to_mouse_vector(player_character->get_floor_coords(), mouse_position_coord);
                player_to_mouse_vector.rotate_xy(shooting_random(gen));
                gffn::physics::Vector3D vec = player_to_mouse_vector.get_vector3d();
                vec = vec * 50;
                gffn::WorldCoordinate projectile_start_coords = player_character->get_floor_coords();
                projectile_start_coords.x += vec.x;
                projectile_start_coords.y += vec.y;
                if (time_since_last_throw > 0.01) {
                    time_since_last_throw = 0;
                    std::unique_ptr<gffn::GFFN_StraightProjectile> projectile = std::make_unique<gffn::GFFN_StraightProjectile>(
                        projectile_start_coords,
                        player_to_mouse_vector,
                        5000,
                        2.0,
                        renderer.get_texture("textures/throwable_explosive.png"),
                        renderer.get_texture("textures/small_shadow.png")
                    );
                    double character_height = player_character->get_physics_controller().get_floor_coords().z;
                    projectile->get_physics_controller().set_height(character_height+100);

                    std::unique_ptr<gffn::GFFN_GameObject> projectile_ptr = std::move(projectile);
                    game_world.add_object(projectile_ptr);
                }
            }

            gffn::WorldCoordinate mouse_coordinates = game_world.get_mouse_position_as_coordinate(renderer);
            player_character->look_at(mouse_coordinates);
            gffn::WorldCoordinate player_coordinates = player_character->get_floor_coords();
            gffn::physics::NormalizedVector3D player_direction(player_coordinates, mouse_coordinates);
            double player_direction_degrees = player_direction.get_xy_direction_in_degrees();
            if (!player_move_key_pressed) {
                // Point the character towards the mouse
                if (player_direction_degrees > 67.5 && player_direction_degrees < 112.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_IDLE_UP);
                }
                else if (player_direction_degrees > 112.5 && player_direction_degrees < 157.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_IDLE_TOP_LEFT);
                }
                else if (player_direction_degrees > 157.5 && player_direction_degrees < 202.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_IDLE_LEFT);
                }
                else if (player_direction_degrees > 202.5 && player_direction_degrees < 247.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_IDLE_BOTTOM_LEFT);
                }
                else if (player_direction_degrees > 247.5 && player_direction_degrees < 292.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_IDLE_DOWN);
                }
                else if (player_direction_degrees > 292.5 && player_direction_degrees < 337) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_IDLE_BOTTOM_RIGHT);
                }
                else if (player_direction_degrees > 337 || player_direction_degrees < 22.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_IDLE_RIGHT);
                }
                else if (player_direction_degrees > 22.5 && player_direction_degrees < 67.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_IDLE_TOP_RIGHT);
                }
            }
            else {
                // Point the character towards the mouse
                if (player_direction_degrees > 67.5 && player_direction_degrees < 112.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_WALK_UP);
                }
                else if (player_direction_degrees > 112.5 && player_direction_degrees < 157.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_WALK_TOP_LEFT);
                }
                else if (player_direction_degrees > 157.5 && player_direction_degrees < 202.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_WALK_LEFT);
                }
                else if (player_direction_degrees > 202.5 && player_direction_degrees < 247.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_WALK_BOTTOM_LEFT);
                }
                else if (player_direction_degrees > 247.5 && player_direction_degrees < 292.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_WALK_DOWN);
                }
                else if (player_direction_degrees > 292.5 && player_direction_degrees < 337) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_WALK_BOTTOM_RIGHT);
                }
                else if (player_direction_degrees > 337 || player_direction_degrees < 22.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_WALK_RIGHT);
                }
                else if (player_direction_degrees > 22.5 && player_direction_degrees < 67.5) {
                    player_character->set_animation_state(gffn::GFFN_CharacterAnimationState::GFFN_ANIMATION_WALK_TOP_RIGHT);
                }
            }
            try {
                game_world.tick(renderer, delta_time_seconds);
            }
            catch (std::exception& e) {
				std::cout << e.what() << std::endl;
				return EXIT_FAILURE;
			}
        }
        SDL_Quit();
    /*}
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (gffn::GFFN_Exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cout << "Unknown exception" << std::endl;
        return EXIT_FAILURE;
    }*/

    return EXIT_SUCCESS;
}
