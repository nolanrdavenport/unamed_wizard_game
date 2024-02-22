#pragma once

#include <gffn_utils.h>
#include <PID.h>

namespace gffn { namespace physics {

typedef struct Vector3D {
    double x;
    double y;
    double z;
    Vector3D(double x, double y, double z) : x(x), y(y), z(z) {}
    Vector3D(WorldCoordinate start, WorldCoordinate end) : x(end.x - start.x), y(end.y - start.y), z(end.z - start.z) {}
    Vector3D() : x(0), y(0), z(0) {}
    bool is_zero() const {
        if (x > 0.001 || x < -0.001 || y > 0.001 || y < -0.001 || z > 0.001 || z < -0.001) {
            return false;
        }
        return true;
    }
    double magnitude() const {
		return sqrt((x * x) + (y * y) + (z * z));
	}
    Vector3D operator+(const Vector3D& other) const {
        return Vector3D(x + other.x, y + other.y, z + other.z);
    }
    Vector3D operator-(const Vector3D& other) const {
        return Vector3D(x - other.x, y - other.y, z - other.z);
    }
    Vector3D operator*(const double& scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }
    Vector3D operator*(const Vector3D& other) const {
        return Vector3D(x * other.x, y * other.y, z * other.z);
    }
    Vector3D operator/(const double& scalar) const {
        return Vector3D(x / scalar, y / scalar, z / scalar);
    }
    Vector3D operator/(const Vector3D& other) const {
        return Vector3D(x / other.x, y / other.y, z / other.z);
    }
} Vector3D;

typedef struct NormalizedVector3D : public Vector3D {
    NormalizedVector3D() : Vector3D(1, 0, 0) {}
    NormalizedVector3D(Vector3D vec) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0, 1);
        verify_normalized_vector(vec.x, vec.y, vec.z); // this causes drift to the bottom right if the passed in vec is all zero.
        double magnitude = sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
        this->x = vec.x / magnitude;
        this->y = vec.y / magnitude;
        this->z = vec.z / magnitude;
    }
    NormalizedVector3D(WorldCoordinate start, WorldCoordinate end) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0, 1);
        double x = end.x - start.x;
        double y = end.y - start.y;
        double z = end.z - start.z;
        verify_normalized_vector(x, y, z);
        double magnitude = sqrt((x * x) + (y * y) + (z * z));
        this->x = ((double)x) / magnitude;
        this->y = ((double)y) / magnitude;
        this->z = ((double)z) / magnitude;
    }
    double get_xy_direction_in_degrees() {
        double y = this->y * -1;

        double radians = atan2(y, x);

        double degrees = (180 * radians) / M_PI;

        return (double)((360 + (int)degrees) % 360);
    }
    void rotate_xy(double degrees) {
        double radians = (degrees * M_PI) / 180;
        double new_x = (x * cos(radians)) - (y * sin(radians));
        double new_y = (x * sin(radians)) + (y * cos(radians));
        x = new_x;
        y = new_y;
    }
    Vector3D get_vector3d() {
		return Vector3D(x, y, z);
	}
} NormalizedVector3D;

// This class will handle everything about positioning, movement, anything physics related.
class ObjectPhysicsController {
    WorldCoordinate floor_coords;
    Vector3D velocity; // in pixels/s where 100 pixels = 1m
    Vector3D net_force; // in kg * pixels/s^2
    double mass; // in kg
    double ground_coef_friction;
    static constexpr double GRAVITY = 980; // in pixels/s^2
    bool gravity_enabled;

public:
    ObjectPhysicsController(WorldCoordinate initial_floor_coords, double ground_coef_friction, bool gravity_enabled=true) : 
    floor_coords(initial_floor_coords), velocity(Vector3D(0, 0, 0)), mass(100), ground_coef_friction(ground_coef_friction), gravity_enabled(gravity_enabled) {}

    // I'm using the concept of momentum for the power of attacks.
    void transfer_momentum(Vector3D momentum) {
        velocity = (momentum + (velocity*mass)) / mass;
	}

    void transfer_momentum(ObjectPhysicsController const& other) {
        Vector3D momentum = other.velocity * other.mass;
        transfer_momentum(momentum);
    }

    bool grounded() const {
        return floor_coords.z < 0.01;
    }

    void add_force(Vector3D force) {
		net_force = (net_force + force);
	}

    WorldCoordinate get_floor_coords() const {
		return floor_coords;
	}

    Vector3D get_net_force() const {
		return net_force;
	}

    Vector3D get_velocity() const {
        return velocity;
    }

    double get_mass() const {
		return mass;
	}

    void set_ground_coef_friction(double ground_coef_friction) {
		this->ground_coef_friction = ground_coef_friction;
	}

    void set_height(double height) {
		floor_coords.z = height;
        velocity.z = 0.0;
	}

    void set_net_force(Vector3D net_force) {
        this->net_force = net_force;
    }

    void set_mass(double mass) { this->mass = mass; }

    void set_velocity(Vector3D velocity) { this->velocity = velocity; }

    void tick(double delta_time_seconds) {
        Vector3D friction_force = velocity * -1 * ground_coef_friction * mass;
        if (grounded()) {
            add_force(friction_force);
		}
        if (gravity_enabled) {
            Vector3D gravity_force = Vector3D(0, 0, -1) * GRAVITY * mass;
            add_force(gravity_force);
        }
        
        Vector3D acceleration = (net_force) / mass;
        acceleration = acceleration * delta_time_seconds;
		velocity = velocity + acceleration;
        
        if(floor_coords.x < 100) {
            floor_coords.x = 101;
            velocity.x = 0;
		}
        if (floor_coords.x > WORLD_GRID_WIDTH * 99) {
            floor_coords.x = (WORLD_GRID_WIDTH * 99) - 1;
            velocity.x = 0;
        }
        if (floor_coords.y < 100) {
			floor_coords.y = 101;
			velocity.y = 0;
		}
        if (floor_coords.y > WORLD_GRID_HEIGHT * 99) {
			floor_coords.y = (WORLD_GRID_HEIGHT * 99) - 1;
			velocity.y = 0;
		}
        floor_coords.x += velocity.x * delta_time_seconds;
        floor_coords.y += velocity.y * delta_time_seconds;
        floor_coords.z += velocity.z * delta_time_seconds;
        if (floor_coords.z < 0.0001) {
            floor_coords.z = 0;
            velocity.z = 0;
        };
        net_force = Vector3D(0, 0, 0);
	}
};

}} // end namespace gffn::physics