add_library(gffn gffn_renderer.cpp gffn_window.cpp "include/gffn_utils.h" "include/gffn_animation.h" "include/gffn_events.h"   "include/gffn_game_world_objects.h" "gffn_utils.cpp" "include/gffn_particles.h" "gffn_events.cpp" "include/gffn_physics.h" "include/PID.h" "PID.cpp" "gffn_game_object.cpp")

target_link_libraries(gffn SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image)
target_include_directories(gffn PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET gffn PROPERTY CXX_STANDARD 20)
endif()