#pragma once

#include "small_types.h"
#include "ivec2.h"
#include "inventory.h"

typedef struct{
	u4 countdown;
	IVEC2 pos;
}BLOCKENTITY;

typedef struct{
	u4 cnt;
	BLOCKENTITY* state;
}BLOCKENTITYHUB;

typedef struct{
	IVEC2 pos;
	SLOT* slot;
}BLOCKENTITYGLOBAL;

typedef struct{
	u4 cnt;
	IVEC2 select;
	BLOCKENTITYGLOBAL* state;
}BLOCKENTITYGLOBALHUB;

extern BLOCKENTITYGLOBALHUB entity_block_global;
extern BLOCKENTITYHUB entity_block;

void entityBlockTick();