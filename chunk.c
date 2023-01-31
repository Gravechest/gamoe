#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "chunk.h"
#include "source.h"
#include "opengl.h"
#include "tmath.h"
#include "entity_light.h"
#include "player.h"
#include "entity_dark.h"
#include "entity_block.h"

CHUNKHUB  chunk;
IVEC2 current_chunk;

u4 coordToChunk(u4 x,u4 y){
	return x*CHUNK_SIZE+y;
}

u4 mapToTileTexture(u4 m_loc){
	IVEC2 crd = {m_loc/SIM_SIZE*TILE_TEXTURE_SIZE,m_loc%SIM_SIZE*TILE_TEXTURE_SIZE};
	return crd.x*SIM_SIZE*TILE_TEXTURE_SIZE+crd.y;
}

u4 coordToTileTexture(u4 x,u4 y){
	return x*SIM_SIZE*TILE_TEXTURE_SURFACE+y*TILE_TEXTURE_SIZE;
}

u4 coordTileTextureToTileTexture(u4 x,u4 y){
	return x*SIM_SIZE*TILE_TEXTURE_SIZE+y;
}

IVEC2 posToTileTextureCoord(VEC2 pos){
	return (IVEC2){pos.x*TILE_TEXTURE_SIZE,pos.y*TILE_TEXTURE_SIZE};
}

u4 posToTileTexture(VEC2 pos){
	IVEC2 crd = {pos.x*TILE_TEXTURE_SIZE,pos.y*TILE_TEXTURE_SIZE};
	return crd.x*SIM_SIZE*TILE_TEXTURE_SIZE+crd.y;
}

void tileTextureGen(VEC2 red,VEC2 green,VEC2 blue,u4 m_loc){
	u4 t_loc =  mapToTileTexture(m_loc);
	for(u4 i = t_loc;i < t_loc+TILE_TEXTURE_SURFACE*SIM_SIZE;i+=SIM_SIZE*TILE_TEXTURE_SIZE){
		for(u4 j = i;j < i+TILE_TEXTURE_SIZE;j++){
			f4 r = tRnd();
			tile_texture_data[j].r = (r+red.x)*red.y;
			tile_texture_data[j].g = (r+green.x)*green.y;
			tile_texture_data[j].b = (r+blue.x)*blue.y;
		}
	}
}

void genMap(IVEC2 crd,u4 offset,u4 depth,f4 value){
	if(!depth){
		u4 m_loc = coordToMap(crd.x,crd.y)+offset;
		map.data[m_loc].health = 0xff;
		if(value > 0.0f){
			map.type[m_loc] = BLOCK_NORMAL;
			tileTextureGen((VEC2){2.0f,64.0f},(VEC2){2.0f,64.0f},(VEC2){2.0f,64.0f},m_loc);
		}
		else if(tRnd() < 1.015f){
			map.type[m_loc] = BLOCK_TREE;
			tileTextureGen((VEC2){1.4f,64.0f},(VEC2){0.4f,32.0f},(VEC2){0.4f,32.0f},m_loc);
		}
		else{
			tileTextureGen((VEC2){2.0f,64.0f},(VEC2){2.0f,64.0f},(VEC2){2.0f,64.0f},m_loc);
		}
		return;
	}
	else{
		crd.x *= 2;
		crd.y *= 2;
		genMap((IVEC2){crd.x  ,crd.y  },offset,depth-1,value+tRnd()-1.5f);
		genMap((IVEC2){crd.x+1,crd.y  },offset,depth-1,value+tRnd()-1.5f);
		genMap((IVEC2){crd.x  ,crd.y+1},offset,depth-1,value+tRnd()-1.5f);
		genMap((IVEC2){crd.x+1,crd.y+1},offset,depth-1,value+tRnd()-1.5f);
	}
}

i4 findChunk(IVEC2 crd){
	union icrd{
		IVEC2 crd;
		u8 id;
	}icrd;
	icrd.crd = crd;
	i4 i = 0,j = chunk.cnt-1;
	while(i <= j){
		i4 k = i + ((j - i) / 2);
		if(chunk.state[k].id == icrd.id){
			return k;
		}
		else if(chunk.state[k].id < icrd.id){
			i = k + 1;
		}
		else{
			j = k - 1;
		}
	}
	return -1;
}

void storeChunk(IVEC2 crd){
	union icrd{
		IVEC2 crd;
		u8 id;
	}icrd;
	icrd.crd = crd;
	u4 offset = (crd.x-current_chunk.x+1)*CHUNK_SIZE+(crd.y-current_chunk.y+1)*SIM_SIZE*CHUNK_SIZE;
	i4 chunkID = findChunk(crd);
	if(chunkID==-1){
		i4 location = 0,bound = chunk.cnt-1;
		while(location <= bound){
			i4 k = location + ((bound - location) / 2);
			if(chunk.state[k].id < icrd.id){
				location = k + 1;
			}
			else{
				bound = k - 1;
			}
		}
		chunk.state[location].chunk.data = HeapAlloc(GetProcessHeap(),0,CHUNK_SIZE*CHUNK_SIZE*sizeof(MAPDATA));
		chunk.state[location].chunk.type = HeapAlloc(GetProcessHeap(),0,CHUNK_SIZE*CHUNK_SIZE);
		for(i4 i = chunk.cnt;i >= location;i--){
			chunk.state[i+1] = chunk.state[i];
		}
		chunk.state[location].crd = icrd.crd;
		for(u4 x = 0;x < CHUNK_SIZE;x++){
			for(u4 y = 0;y < CHUNK_SIZE;y++){
				chunk.state[location].chunk.type[coordToChunk(x,y)] = map.type[coordToMap(x,y)+offset];
				chunk.state[location].chunk.data[coordToChunk(x,y)] = map.data[coordToMap(x,y)+offset];
			}
		}
		chunk.cnt++;
	}
	else{
		for(u4 x = 0;x < CHUNK_SIZE;x++){
			for(u4 y = 0;y < CHUNK_SIZE;y++){
				chunk.state[chunkID].chunk.data[coordToChunk(x,y)] = map.data[coordToMap(x,y)+offset];
				chunk.state[chunkID].chunk.type[coordToChunk(x,y)] = map.type[coordToMap(x,y)+offset];
			}
		}
	}
}

void loadChunk(IVEC2 crd){
	u4 chunkID = findChunk((IVEC2){current_chunk.x+crd.x,current_chunk.y+crd.y});
	u4 offset = (crd.x+1)*CHUNK_SIZE + (crd.y+1)*SIM_SIZE*CHUNK_SIZE;
	if(chunkID==-1){
		genMap((IVEC2){0,0},offset,7,-1.0f);
	}
	else{
		for(u4 x = 0;x < CHUNK_SIZE;x++){
			for(u4 y = 0;y < CHUNK_SIZE;y++){
				u4 m_loc = y*SIM_SIZE+x+offset;
				map.data[m_loc] = chunk.state[chunkID].chunk.data[y*CHUNK_SIZE+x];
				map.type[m_loc] = chunk.state[chunkID].chunk.type[y*CHUNK_SIZE+x];
				switch(map.type[coordToMap(x,y)+offset]){
				case BLOCK_SPRINKLER:
					entity_block.state[entity_block.cnt].pos = (IVEC2){m_loc/SIM_SIZE,m_loc%SIM_SIZE};
					entity_block.state[entity_block.cnt++].countdown = 0;
					break;
				}
			}
		}
	}
}

void moveEntities(f4 direction,u4 axis){
	camera.pos.a[axis] += direction;
	player.pos.a[axis] += direction;
	CHUNK_ENTITY_MOVE(entity_dark,ENEMY_SIZE);
	CHUNK_ENTITY_MOVE(entity_light,2.0f);

	for(u4 i = 0;i < entity_block.cnt;i++){
		entity_block.state[i].pos.a[axis] += direction;
		if(entity_block.state[i].pos.a[axis]<0||entity_block.state[i].pos.a[axis]>=SIM_SIZE){
			ENTITY_REMOVE(entity_block,i);
		}
	}
}

void worldLoadSpawn(){
	for(u4 x = 0;x <= CHUNK_SIZE*2;x+=CHUNK_SIZE){
		for(u4 y = 0;y <= SIM_SIZE*CHUNK_SIZE*2;y+=SIM_SIZE*CHUNK_SIZE){
			genMap((IVEC2){0,0},x+y,7,-1.0f);
		}
	}
	//make sure the spawnarea is clean
	for(u4 x = player.pos.x - 3.0f;x < player.pos.x + 3.0f;x++){
		for(u4 y = player.pos.y - 3.0f;y < player.pos.y + 3.0f;y++){
			map.type[coordToMap(x,y)] = BLOCK_AIR;
		}
	}
	for(u4 i = 0;i < SIM_SIZE_SURFACE;i++){
		switch(map.type[i]){
		case BLOCK_SPRINKLER:
			entity_block.state[entity_block.cnt].pos = (IVEC2){i/SIM_SIZE,i%SIM_SIZE};
			entity_block.state[entity_block.cnt++].countdown = 0;
			break;
		}
	}
}

void worldLoadEast(){
	for(i4 i = current_chunk.y-1;i <= current_chunk.y+1;i++){
		storeChunk((IVEC2){current_chunk.x-1,i});
	}
	moveEntities(-CHUNK_SIZE,VEC2_X);
	for(u4 x = 0;x < SIM_SIZE;x++){
		for(u4 y = 0;y < CHUNK_SIZE*2;y++){
			map.type[x*SIM_SIZE+y] = map.type[x*SIM_SIZE+y+CHUNK_SIZE];
			map.data[x*SIM_SIZE+y] = map.data[x*SIM_SIZE+y+CHUNK_SIZE];
		}
	}
	for(u4 x = 0;x < SIM_SIZE*TILE_TEXTURE_SIZE;x++){
		for(u4 y = 0;y < CHUNK_SIZE*TILE_TEXTURE_SIZE*2;y++){
			tile_texture_data[x*SIM_SIZE*TILE_TEXTURE_SIZE+y] = tile_texture_data[x*SIM_SIZE*TILE_TEXTURE_SIZE+y+CHUNK_SIZE*TILE_TEXTURE_SIZE];
		}
	}
	current_chunk.x++;
	for(i4 y = -1;y <= 1;y++) loadChunk((IVEC2){1,y});
}

void worldLoadWest(){
	for(i4 i = current_chunk.y-1;i <= current_chunk.y+1;i++){
		storeChunk((IVEC2){current_chunk.x+1,i});
	}
	moveEntities(CHUNK_SIZE,VEC2_X);
	for(i4 x = SIM_SIZE-1;x >= 0;x--){
		for(i4 y = CHUNK_SIZE*2-1;y >= 0;y--){
			map.data[x*SIM_SIZE+y+CHUNK_SIZE] = map.data[x*SIM_SIZE+y];
			map.type[x*SIM_SIZE+y+CHUNK_SIZE]   = map.type[x*SIM_SIZE+y];
		}
	}
	for(i4 x = SIM_SIZE*TILE_TEXTURE_SIZE-1;x >= 0;x--){
		for(i4 y = CHUNK_SIZE*TILE_TEXTURE_SIZE*2-1;y >= 0;y--){
			tile_texture_data[x*SIM_SIZE*TILE_TEXTURE_SIZE+y+CHUNK_SIZE*TILE_TEXTURE_SIZE] = tile_texture_data[x*SIM_SIZE*TILE_TEXTURE_SIZE+y];
		}
	}
	current_chunk.x--;
	for(i4 y = -1;y <= 1;y++) loadChunk((IVEC2){-1,y});
}

void worldLoadNorth(){
	for(i4 i = current_chunk.x-1;i <= current_chunk.x+1;i++){
		storeChunk((IVEC2){i,current_chunk.y-1});
	}
	moveEntities(-CHUNK_SIZE,VEC2_Y);
	memcpy(tile_texture_data,tile_texture_data+SIM_SIZE*CHUNK_SIZE*TILE_TEXTURE_SURFACE,SIM_SIZE*CHUNK_SIZE*TILE_TEXTURE_SURFACE);
	memcpy(tile_texture_data+SIM_SIZE*CHUNK_SIZE*TILE_TEXTURE_SURFACE,tile_texture_data+SIM_SIZE*CHUNK_SIZE*TILE_TEXTURE_SURFACE*2,SIM_SIZE*CHUNK_SIZE*TILE_TEXTURE_SURFACE);
	memcpy(map.type,map.type+SIM_SIZE*CHUNK_SIZE,SIM_SIZE*CHUNK_SIZE);
	memcpy(map.type+SIM_SIZE*CHUNK_SIZE,map.type+SIM_SIZE*CHUNK_SIZE*2,SIM_SIZE*CHUNK_SIZE);
	current_chunk.y++;
	for(i4 x = -1;x <= 1;x++) loadChunk((IVEC2){x,1});
}

void worldLoadSouth(){
	for(i4 i = current_chunk.x-1;i <= current_chunk.x+1;i++){
		storeChunk((IVEC2){i,current_chunk.y+1});
	}
	moveEntities(CHUNK_SIZE,VEC2_Y);
	memcpy(tile_texture_data+SIM_SIZE*CHUNK_SIZE*TILE_TEXTURE_SURFACE*2,tile_texture_data+SIM_SIZE*CHUNK_SIZE*TILE_TEXTURE_SURFACE,SIM_SIZE*CHUNK_SIZE*TILE_TEXTURE_SURFACE);
	memcpy(tile_texture_data+SIM_SIZE*CHUNK_SIZE*TILE_TEXTURE_SURFACE,tile_texture_data,SIM_SIZE*CHUNK_SIZE*TILE_TEXTURE_SURFACE);
	memcpy(map.type+SIM_SIZE*CHUNK_SIZE*2,map.type+SIM_SIZE*CHUNK_SIZE,SIM_SIZE*CHUNK_SIZE);
	memcpy(map.type+SIM_SIZE*CHUNK_SIZE,map.type,SIM_SIZE*CHUNK_SIZE);
	current_chunk.y--;
	for(i4 x = -1;x <= 1;x++) loadChunk((IVEC2){x,-1});
}