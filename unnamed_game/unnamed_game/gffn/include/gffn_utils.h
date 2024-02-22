#pragma once

#include <cmath>
#include <SDL_stdinc.h>
#include <limits>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <unordered_set>
#include <SDL.h>
#include <format>

#include <gffn_exception.h>

namespace gffn {

static constexpr int RENDERER_LOGICAL_WIDTH = 1920;
static constexpr int RENDERER_LOGICAL_HEIGHT = 1080;

static constexpr int WORLD_GRID_WIDTH = 100;
static constexpr int WORLD_GRID_HEIGHT = 100;

typedef enum : unsigned int {
    GFFN_OBJECT_TYPE_CHARACTER = 0,
    GFFN_OBJECT_TYPE_NPC,
    GFFN_OBJECT_TYPE_INANIMATE_OBJECT,
    GFFN_OBJECT_TYPE_UI_OBJECT,
    GFFN_OBJECT_TYPE_THROWABLE_EXPLOSIVE,
    GFFN_OBJECT_TYPE_STRAIGHT_PROJECTILE,
    GFFN_OBJECT_TYPE_PARTICLE,
    GFFN_OBJECT_TYPE_ENVIRONMENTAL_OBJECT,
    GFFN_OBJECT_TYPE_DISMEMBERED_BODY_PART,
    GFFN_OBJECT_TYPE_GROUND_OBJECT,
    GFFN_OBJECT_TYPE_END
} GFFN_ObjectType;

typedef enum {
    GFFN_TEXTURE_THROWABLE_EXPLOSIVE,
    GFFN_TEXTURE_CHARACTER_SHADOW,
    GFFN_TEXTURE_SMALL_SHADOW,
    GFFN_TEXTURE_GROUND_YELLOW_FLOWERS,
    GFFN_TEXTURE_GROUND_SANDSTONE_BRICKS,
    GFFN_TEXTURE_PLAYER_POINTER,
    GFFN_TEXTURE_EXPLOSION_PARTICLE
} GFFN_TextureEnum;

typedef struct Range {
    double min;
    double max;
    Range(double min, double max) : min(min), max(max) {}
    double get_rand_in_range() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(min, max);
        return dis(gen);
    }
} Range;

typedef struct Vector2D {
    double x;
    double y;
    Vector2D(double x, double y) : x(x), y(y) {}
    Vector2D() : x(0), y(0) {}

} Vector2D;

class GFFN_MultiImage {
    SDL_Texture* texture;
    std::shared_ptr<SDL_Rect> source_rect;
    static constexpr int SINGLE_IMAGE_WIDTH = 25;
    static constexpr int SINGLE_IMAGE_HEIGHT = 25;
    int number_of_images;
public:
    GFFN_MultiImage(SDL_Texture* texture) : texture(texture) {
        int width, height;
        SDL_QueryTexture(texture, NULL, NULL, &width, &height);
        if (width % SINGLE_IMAGE_WIDTH != 0) {
            throw GFFN_Exception(std::format("Animation texture width is not a multiple of {}.", SINGLE_IMAGE_WIDTH));
        }
        if (height % SINGLE_IMAGE_HEIGHT != 0) {
            throw GFFN_Exception(std::format("Animation texture height is not a multiple of {}.", SINGLE_IMAGE_HEIGHT));
        }
        number_of_images = width / SINGLE_IMAGE_WIDTH;
        source_rect = std::make_shared<SDL_Rect>();
    }
    std::pair<SDL_Texture*, SDL_Rect*> get_image(int image_index) {
        if (image_index >= number_of_images) {
            throw GFFN_Exception(std::format("Image number {} is out of range.", image_index));
        }
        *source_rect = { image_index * SINGLE_IMAGE_WIDTH, 0, SINGLE_IMAGE_WIDTH, SINGLE_IMAGE_HEIGHT };
        return std::make_pair(texture, source_rect.get());
    }
};

// There's issue with things being very close to zero, so this function will make sure that doesn't happen.
static void verify_normalized_vector(double &x, double &y, double &z) {
    int num_zeros = 0;
    if (FP_ZERO == fpclassify(x)) {
        //x = 0.0000001;
        num_zeros++;
    }
    if (FP_ZERO == fpclassify(y)) {
        //y = 0.0000001;
        num_zeros++;
    }
    if (FP_ZERO == fpclassify(z)) {
        //z = 0.0000001;
        num_zeros++;
    }
    if (num_zeros == 3) {
		throw std::runtime_error("Normalized vector was created with all three values as zero, which causes issues.");
	}
}

typedef struct WorldCoordinate {
    double x;
    double y;
    double z;

    WorldCoordinate(double x, double y, double z) : x(x), y(y), z(z) {}
    WorldCoordinate() : x(0), y(0), z(0) {}
    WorldCoordinate(const WorldCoordinate& other) : x(other.x), y(other.y), z(other.z) {}
    double distance_from(WorldCoordinate from) const {
        double x_component = from.x - x;
        double y_component = from.y - y;
        double z_component = from.z - z;
        //unzero_3D(x_component, y_component, z_component);

        return sqrt((x_component * x_component) + (y_component * y_component) + (z_component * z_component));
    }

    double distance_from_squared_xy(WorldCoordinate from) const {
        double x_component = from.x - x;
		double y_component = from.y - y;
		//unzero_2D(x_component, y_component);

		return (x_component * x_component) + (y_component * y_component);
    }

    WorldCoordinate get_coordinate_between(WorldCoordinate other) {
        return WorldCoordinate((x + other.x) / 2.0, (y + other.y) / 2.0, (z + other.z) / 2.0);
    }
    WorldCoordinate get_coordinate_between_percent_from_source(WorldCoordinate other, double percent) const {
        double x_component = other.x - x;
        double y_component = other.y - y;
        double z_component = other.z - z;
        percent /= 100;
        return WorldCoordinate(x + (x_component * percent), y + (y_component * percent), z + (z_component * percent));
    }
} WorldCoordinate;

typedef struct WorldCoordinateInt2D {
	int x;
	int y;
	WorldCoordinateInt2D(int x, int y) : x(x), y(y) {}
	WorldCoordinateInt2D() : x(0), y(0) {}
	WorldCoordinateInt2D(const WorldCoordinateInt2D& other) : x(other.x), y(other.y) {}
    WorldCoordinateInt2D(const WorldCoordinate& other) : x((int)other.x), y((int)other.y) {}
} WorldCoordinateInt2D;

static constexpr int FLOOR_COORDS_Y_OFFSET = -20;

static WorldCoordinateInt2D calculate_top_left_coords_from_floor_coords(WorldCoordinate floor_coords, int width, int height) {
    WorldCoordinateInt2D top_left_coords((int)floor_coords.x - (width / 2), (int)floor_coords.y - height - FLOOR_COORDS_Y_OFFSET);
    return top_left_coords;
}

}  // end namespace gffn