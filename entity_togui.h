#pragma once

#include "small_types.h"
#include "vec2.h"

typedef struct{
	VEC2 pos;
	VEC2 vel;
	VEC2 dst;
	u4 type;
	u4 inventory_slot;
	f4 size;
}ENTITYTOGUI;

typedef struct{
	u4 cnt;
	ENTITYTOGUI* state;
}ENTITYTOGUIHUB;

void entityToGuiTick();
void entityToGuiSpawn(VEC2 pos,VEC2 dst,f4 size,u4 slot,u4 type);

extern ENTITYTOGUIHUB entity_togui;