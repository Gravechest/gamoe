#pragma once

#include "small_types.h"
#include "vec2.h"
#include "vec3.h"

#define ENEMY_SIZE 1.7f

typedef struct{
	VEC2 vel;
	VEC2 pos;
	VEC3 luminance;
	u4 health;
	u1 type;
	u1 aggressive;
}ENEMY;

typedef struct{
	u4 cnt;
	ENEMY* state;
}ENEMYHUB;

void entityDarkTick();

extern ENEMYHUB entity_dark;