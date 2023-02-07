#pragma once

#include "small_types.h"
#include "vec2.h"
#include "inventory.h"

typedef struct{
	VEC2 pos;
	VEC2 vel;
	VEC2 dst;
	ITEM item;
	u4 inventory_slot;
	f4 size;
}ENTITYTOGUI;

typedef struct{
	u4 cnt;
	ENTITYTOGUI* state;
}ENTITYTOGUIHUB;

void entityToGuiTick();
void entityToGuiSpawn(VEC2 pos,VEC2 dst,f4 size,u4 slot);

extern ENTITYTOGUIHUB entity_togui;