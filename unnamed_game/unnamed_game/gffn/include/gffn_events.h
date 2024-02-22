#pragma once

#include <vector>
#include <queue>
#include <variant>
#include <gffn_utils.h>

namespace gffn { 

class GFFN_StraightProjectile;
class GFFN_GridObject;

namespace events {
typedef struct ExplosionEvent {
	WorldCoordinate coordinates;
	int power;
	long int blast_radius;
	int min_damage;
	std::vector<std::tuple<long unsigned int, GFFN_ObjectType>> objects_hit;
} ExplosionEvent;


typedef struct ProjectileHitEvent {
	GFFN_StraightProjectile* const projectile;
	GFFN_GridObject* const object;
} ProjectileHitEvent;

//typedef struct ProjectileHitEvent {
//	std::pair<long unsigned int, GFFN_ObjectType> projectile_info;
//	std::pair<long unsigned int, GFFN_ObjectType> object_info;
//} ProjectileHitEvent;

typedef std::queue<std::variant<ExplosionEvent, ProjectileHitEvent>> EventQueue;

extern EventQueue event_queue;
}} // end namespace gffn::events