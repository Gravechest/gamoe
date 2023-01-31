#pragma once

#include "small_types.h"
#include "ivec2.h"

typedef struct{
	u4 countdown;
	IVEC2 pos;
}BLOCKENTITY;

typedef struct{
	u4 cnt;
	BLOCKENTITY* state;
}BLOCKENTITYHUB;

extern BLOCKENTITYHUB entity_block;

void entityBlockTick();