# CMakeList.txt : CMake project for unnamed_game, include source and define
# project specific logic here.
#

find_package(SDL2 REQUIRED)
find_package(SDL2_IMAGE REQUIRED)

add_subdirectory(gffn)

# Add source to this project's executable.
add_executable (unnamed_game "unnamed_game.cpp" "unnamed_game.h")

add_custom_command(TARGET unnamed_game PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_CURRENT_LIST_DIR}/textures/ ${CMAKE_CURRENT_BINARY_DIR}/textures)

target_link_libraries(unnamed_game SDL2::SDL2 SDL2::SDL2main SDL2_image::SDL2_image gffn)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET unnamed_game PROPERTY CXX_STANDARD 20)
endif()

include(CMakePrintHelpers)

cmake_print_variables(CMAKE_CURRENT_LIST_DIR)
cmake_print_variables(CMAKE_CURRENT_BINARY_DIR)


# TODO: Add tests and install targets if needed.
