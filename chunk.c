#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "chunk.h"
#include "source.h"

CHUNKHUB  chunk;
IVEC2 chunkPointer;

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
	u4 offset = (crd.x-chunkPointer.x+1)*RES+(crd.y-chunkPointer.y+1)*MAP*RES;
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
	u4 chunkID = findChunk((IVEC2){chunkPointer.x+crd.x,chunkPointer.y+crd.y});
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

void worldLoadEast(){
	for(i4 i = chunkPointer.y-1;i <= chunkPointer.y+1;i++){
		storeChunk((IVEC2){chunkPointer.x-1,i});
	}
	camera.x -= RES;
	player.pos.x -= RES;
	for(u4 i = 0;i < enemy.cnt;i++) enemy.state[i].pos.x -= RES;
	for(u4 i = 0;i < bullet.cnt;i++) bullet.state[i].pos.x -= RES;
	for(u4 x = 0;x < MAP;x++){
		for(u4 y = 0;y < RES*2;y++){
			map[x*MAP+y] = map[x*MAP+y+RES];
		}
	}
	chunkPointer.x++;
	for(i4 y = -1;y <= 1;y++) loadChunk((IVEC2){1,y});
}

void worldLoadWest(){
	for(i4 i = chunkPointer.y-1;i <= chunkPointer.y+1;i++){
		storeChunk((IVEC2){chunkPointer.x+1,i});
	}
	camera.x += RES;
	player.pos.x += RES;
	for(u4 i = 0;i < enemy.cnt;i++) enemy.state[i].pos.x += RES;
	for(u4 i = 0;i < bullet.cnt;i++) bullet.state[i].pos.x += RES;
	for(i4 x = MAP-1;x >= 0;x--){
		for(i4 y = RES*2-1;y >= 0;y--){
			map[x*MAP+y+RES] = map[x*MAP+y];
		}
	}
	chunkPointer.x--;
	for(i4 y = -1;y <= 1;y++) loadChunk((IVEC2){-1,y});
}

void worldLoadNorth(){
	for(i4 i = chunkPointer.x-1;i <= chunkPointer.x+1;i++){
		storeChunk((IVEC2){i,chunkPointer.y-1});
	}
	camera.y -= RES;
	player.pos.y -= RES;
	memcpy(map,map+MAP*RES,MAP*RES);
	memcpy(map+MAP*RES,map+MAP*RES*2,MAP*RES);
	for(u4 i = 0;i < enemy.cnt;i++) enemy.state[i].pos.y -= RES;
	for(u4 i = 0;i < bullet.cnt;i++) bullet.state[i].pos.y -= RES;
	chunkPointer.y++;
	for(i4 x = -1;x <= 1;x++) loadChunk((IVEC2){x,1});
}

void worldLoadSouth(){
	for(i4 i = chunkPointer.x-1;i <= chunkPointer.x+1;i++){
		storeChunk((IVEC2){i,chunkPointer.y+1});
	}
	camera.y += RES;
	player.pos.y += RES;
	for(u4 i = 0;i < enemy.cnt;i++) enemy.state[i].pos.y += RES;
	for(u4 i = 0;i < bullet.cnt;i++) bullet.state[i].pos.y += RES;
	memcpy(map+MAP*RES*2,map+MAP*RES,MAP*RES);
	memcpy(map+MAP*RES,map,MAP*RES);
	chunkPointer.y--;
	for(i4 x = -1;x <= 1;x++) loadChunk((IVEC2){x,-1});
}