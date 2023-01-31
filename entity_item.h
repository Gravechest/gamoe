#pragma once

#include "small_types.h"
#include "vec2.h"
#include "vec3.h"

#define ENTITY_ITEM_SIZE 1.4f
#define ENTITY_BLOCK_PARTICLE_SIZE 0.25f

enum{
	ENTITY_ITEM,
	ENTITY_BLOCK_PARTICLE
};

typedef struct{
	u1 type;
	u1 durability;
}ITEM;

typedef struct{
	VEC3 luminance;
	VEC2 pos;
	VEC2 vel;
	f4 size;
	u1 type;
	ITEM item;
	union{
		u2 pickup_countdown;
		u2 health;
	};
}ITEM_ENTITY;

typedef struct{
	u4 cnt;
	ITEM_ENTITY* state;
}ITEM_ENTITYHUB;

i4 inventoryEmptySlot();
void entityItemTick();
void blockParticleSpawn(VEC2 pos,VEC2 vel);
void itemEntitySpawn(VEC2 pos,VEC2 vel,ITEM item);
void itemEntitySpawnNew(VEC2 pos,VEC2 vel,u4 item_type);

extern ITEM_ENTITYHUB entity_item;