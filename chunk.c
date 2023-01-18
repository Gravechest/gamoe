#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "chunk.h"
#include "source.h"
#include "opengl.h"

CHUNKHUB  chunk;
IVEC2 current_chunk;

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
	u4 offset = (crd.x-current_chunk.x+1)*RES+(crd.y-current_chunk.y+1)*MAP*RES;
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
		chunk.state[location].chunk = HeapAlloc(GetProcessHeap(),0,RES*RES);
		for(i4 i = chunk.cnt;i >= location;i--){
			chunk.state[i+1] = chunk.state[i];
		}
		chunk.state[location].crd = icrd.crd;
		for(u4 x = 0;x < RES;x++){
			for(u4 y = 0;y < RES;y++){
				chunk.state[location].chunk[x*RES+y] = map[y*MAP+x+offset];
			}
		}
		chunk.cnt++;
	}
	else{
		for(u4 x = 0;x < RES;x++){
			for(u4 y = 0;y < RES;y++){
				chunk.state[chunkID].chunk[x*RES+y] = map[y*MAP+x+offset];
			}
		}
	}
}

void loadChunk(IVEC2 crd){
	u4 chunkID = findChunk((IVEC2){current_chunk.x+crd.x,current_chunk.y+crd.y});
	u4 offset = (crd.x+1)*RES + (crd.y+1)*MAP*RES;
	if(chunkID==-1){
		genMap((IVEC2){0,0},offset,7,-1.0f);
	}
	else{
		for(u4 x = 0;x < RES;x++){
			for(u4 y = 0;y < RES;y++){
				map[y*MAP+x+offset] = chunk.state[chunkID].chunk[y*RES+x];
			}
		}
	}
}

void moveEntities(f4 direction,u4 axis){
	camera.pos.a[axis] += direction;
	player.pos.a[axis] += direction;
	CHUNK_ENTITY_MOVE(enemy,ENEMY_SIZE);
	CHUNK_ENTITY_MOVE(bullet,BULLET_SIZE);
	CHUNK_ENTITY_MOVE(particle,2.0f);
}

void worldLoadEast(){
	for(i4 i = current_chunk.y-1;i <= current_chunk.y+1;i++){
		storeChunk((IVEC2){current_chunk.x-1,i});
	}
	moveEntities(-RES,VEC2_X);
	for(u4 x = 0;x < MAP;x++){
		for(u4 y = 0;y < RES*2;y++){
			map[x*MAP+y] = map[x*MAP+y+RES];
		}
	}
	current_chunk.x++;
	for(i4 y = -1;y <= 1;y++) loadChunk((IVEC2){1,y});
}

void worldLoadWest(){
	for(i4 i = current_chunk.y-1;i <= current_chunk.y+1;i++){
		storeChunk((IVEC2){current_chunk.x+1,i});
	}
	moveEntities(RES,VEC2_X);
	for(i4 x = MAP-1;x >= 0;x--){
		for(i4 y = RES*2-1;y >= 0;y--){
			map[x*MAP+y+RES] = map[x*MAP+y];
		}
	}
	current_chunk.x--;
	for(i4 y = -1;y <= 1;y++) loadChunk((IVEC2){-1,y});
}

void worldLoadNorth(){
	for(i4 i = current_chunk.x-1;i <= current_chunk.x+1;i++){
		storeChunk((IVEC2){i,current_chunk.y-1});
	}
	moveEntities(-RES,VEC2_Y);
	memcpy(map,map+MAP*RES,MAP*RES);
	memcpy(map+MAP*RES,map+MAP*RES*2,MAP*RES);
	current_chunk.y++;
	for(i4 x = -1;x <= 1;x++) loadChunk((IVEC2){x,1});
}

void worldLoadSouth(){
	for(i4 i = current_chunk.x-1;i <= current_chunk.x+1;i++){
		storeChunk((IVEC2){i,current_chunk.y+1});
	}
	moveEntities(RES,VEC2_Y);
	memcpy(map+MAP*RES*2,map+MAP*RES,MAP*RES);
	memcpy(map+MAP*RES,map,MAP*RES);
	current_chunk.y--;
	for(i4 x = -1;x <= 1;x++) loadChunk((IVEC2){x,-1});
}