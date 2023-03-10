#pragma once

#include "small_types.h"
#include "vec2.h"
#include "vec3.h"

#define LOOT_ENERGY_COLOR (VEC3){0.6f,0.6f,0.12f}
#define LOOT_HEALTH_COLOR (VEC3){0.7f,0.2f,0.2f}
#define LOOT_SCRAP_COLOR  (VEC3){0.12f,0.24f,0.6f}
#define BOMB_COLOR (VEC3){0.02f,0.005f,0.005f}

#define LOOT_ENERGY_COLOR_PARENT (VEC3){LOOT_ENERGY_COLOR.r/30.0f,LOOT_ENERGY_COLOR.g/30.0f,LOOT_ENERGY_COLOR.b/30.0f}
#define LOOT_HEALTH_COLOR_PARENT (VEC3){LOOT_HEALTH_COLOR.r/30.0f,LOOT_HEALTH_COLOR.g/30.0f,LOOT_HEALTH_COLOR.b/30.0f}
#define LOOT_SCRAP_COLOR_PARENT  (VEC3){LOOT_SCRAP_COLOR.r/30.0f,LOOT_SCRAP_COLOR.g/30.0f,LOOT_SCRAP_COLOR.b/30.0f}

#define LOOT_ENERGY_COLOR_INFANT (VEC3){LOOT_ENERGY_COLOR.r/200.0f,LOOT_ENERGY_COLOR.g/200.0f,LOOT_ENERGY_COLOR.b/200.0f}
#define LOOT_HEALTH_COLOR_INFANT (VEC3){LOOT_HEALTH_COLOR.r/200.0f,LOOT_HEALTH_COLOR.g/200.0f,LOOT_HEALTH_COLOR.b/200.0f}
#define LOOT_SCRAP_COLOR_INFANT  (VEC3){LOOT_SCRAP_COLOR.r/200.0f,LOOT_SCRAP_COLOR.g/200.0f,LOOT_SCRAP_COLOR.b/200.0f}

enum{
	PARTICLE_NORMAL,
	LOOT_INFANT,
	LOOT_PARENT,
	ENTITY_BOMB
};

enum{
	LOOT_ENERGY,
	LOOT_HEALTH,
	LOOT_SCRAP
};

typedef struct{
	VEC3 color;
	VEC2 pos;
	VEC2 vel;
	u4 health;
	f4 size;
	u1 type;
	u1 loot_type;
}PARTICLE;

typedef struct{
	u4 cnt;
	PARTICLE* state;
}PARTICLEHUB;

extern PARTICLEHUB entity_light;

void entityLightTick();
void particleSpawn(VEC3 color,VEC2 position,VEC2 velocity,f4 size,u4 health);
void spawnLootOrb(u1 type,VEC2 direction,VEC2 position);