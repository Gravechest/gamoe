#pragma once

#include "small_types.h"
#include "ivec2.h"

#define CHUNK_ENTITY_MOVE(TYPE,SIZE) for(u4 J = 0;J < TYPE.cnt;J++){                                     \
		TYPE.state[J].pos.a[axis] += direction;                                                          \
		if(TYPE.state[J].pos.a[axis] < SIZE || TYPE.state[J].pos.a[axis] > SIM_SIZE-SIZE-1.0f){               \
			ENTITY_REMOVE(TYPE,J)                                                                        \
		}                                                                                                \
	}

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
extern IVEC2 current_chunk;