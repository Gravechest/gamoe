#pragma once

#include "small_types.h"
#include "ivec2.h"

typedef struct{
	union{
		IVEC2 crd;
		u8 id;
	};
	u1* chunk;
}CHUNK;

typedef struct{
	CHUNK* state;
	u4 cnt;
}CHUNKHUB;

void worldLoadEast();
void worldLoadWest();
void worldLoadNorth();
void worldLoadSouth();

extern CHUNKHUB chunk;