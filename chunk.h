#pragma once

#include "small_types.h"
#include "ivec2.h"
#include "source.h"

#define CHUNK_ENTITY_MOVE(TYPE,SIZE) for(u4 J = 0;J < TYPE.cnt;J++){                                     \
		TYPE.state[J].pos.a[axis] += direction;                                                          \
		if(TYPE.state[J].pos.a[axis] < SIZE || TYPE.state[J].pos.a[axis] > SIM_SIZE-SIZE-1.0f){          \
			ENTITY_REMOVE(TYPE,J)                                                                        \
		}                                                                                                \
	}

typedef struct{
	union{
		IVEC2 crd;
		u8 id;
	};
	MAP chunk;
	RGB* tile_texture;
}CHUNK;

typedef struct{
	CHUNK* state;
	u4 cnt;
}CHUNKHUB;

void worldLoadEast();
void worldLoadWest();
void worldLoadNorth();
void worldLoadSouth();
void worldLoadSpawn();
u4 posToTileTexture(VEC2 pos);
IVEC2 posToTileTextureCoord(VEC2 pos);
u4 coordToTileTexture(u4 x,u4 y);
void tileTextureGen(VEC2 red,VEC2 green,VEC2 blue,u4 m_loc);
u4 coordTileTextureToTileTexture(u4 x,u4 y);

extern CHUNKHUB chunk;
extern IVEC2 current_chunk;