#pragma once

#include <gffn_game_object.h>

namespace gffn { namespace particles {

struct particle_info {
	int num_particles;
	int num_particles_per_tick;
	int num_particles_per_second;
	int num_particles_per_second_variance;
	int num_particles_per_tick_variance;
	int particle_lifetime;
	int particle_lifetime_variance;
	int particle_size;
	int particle_size_variance;
	int particle_speed;
	int particle_speed_variance;
	int particle_direction;
	int particle_direction_variance;
	int particle_color;
	int particle_color_variance;
};

class Particle : public GFFN_GridObject {
public:
	Particle(WorldCoordinate floor_coords, bool fade=false, bool shrink=false) : GFFN_GridObject(GFFN_OBJECT_TYPE_PARTICLE, 1, 1, floor_coords, nullptr) {}
	void tick(long unsigned int delta_time) {
		GFFN_GridObject::tick(delta_time);
	}
};

template <int num_particles>
class ParticleEmitter : public GFFN_GameObject {
public:
	ParticleEmitter(WorldCoordinate floor_coords, bool fade=false, bool shrink=false) : GFFN_GameObject(GFFN_OBJECT_TYPE_PARTICLE_EMITTER, floor_coords, nullptr) {}
	void tick(long unsigned int delta_time) {
		GFFN_GameObject::tick(delta_time);
	}
};

}} // end namespace gffn