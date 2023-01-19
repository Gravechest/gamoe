#include <windows.h>
#include <stdio.h>
#include <string.h>

#include "chunk.h"
#include "source.h"
#include "opengl.h"
#include "tmath.h"

CHUNKHUB  chunk;
IVEC2 current_chunk;

void genMap(IVEC2 crd,u4 offset,u4 depth,f4 value){
	if(!depth){
		if(value > 0.0f) map[crd.x*SIM_SIZE+crd.y+offset] = BLOCK_NORMAL;
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
		chunk.state[location].chunk = HeapAlloc(GetProcessHeap(),0,CHUNK_SIZE*CHUNK_SIZE);
		for(i4 i = chunk.cnt;i >= location;i--){
			chunk.state[i+1] = chunk.state[i];
		}
		chunk.state[location].crd = icrd.crd;
		for(u4 x = 0;x < CHUNK_SIZE;x++){
			for(u4 y = 0;y < CHUNK_SIZE;y++){
				chunk.state[location].chunk[x*CHUNK_SIZE+y] = map[y*SIM_SIZE+x+offset];
			}
		}
		chunk.cnt++;
	}
	else{
		for(u4 x = 0;x < CHUNK_SIZE;x++){
			for(u4 y = 0;y < CHUNK_SIZE;y++){
				chunk.state[chunkID].chunk[x*CHUNK_SIZE+y] = map[y*SIM_SIZE+x+offset];
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
				map[m_loc] = chunk.state[chunkID].chunk[y*CHUNK_SIZE+x];
				switch(map[y*SIM_SIZE+x+offset]){
				case BLOCK_SPRINKLER:
					block_entity.state[block_entity.cnt].pos = (IVEC2){m_loc/SIM_SIZE,m_loc%SIM_SIZE};
					block_entity.state[block_entity.cnt++].countdown = 0;
					break;
				}
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

	for(u4 i = 0;i < block_entity.cnt;i++){
		block_entity.state[i].pos.a[axis] += direction;
		if(block_entity.state[i].pos.a[axis]<0||block_entity.state[i].pos.a[axis]>=SIM_SIZE){
			ENTITY_REMOVE(block_entity,i);
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
			map[x*SIM_SIZE+y] = 0;
		}
	}
	map[SIM_SIZE*SIM_SIZE/2+SIM_SIZE/2] = BLOCK_SPRINKLER;
	for(u4 i = 0;i < SIM_SIZE_SURFACE;i++){
		switch(map[i]){
		case BLOCK_SPRINKLER:
			block_entity.state[block_entity.cnt].pos = (IVEC2){i/SIM_SIZE,i%SIM_SIZE};
			block_entity.state[block_entity.cnt++].countdown = 0;
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
			map[x*SIM_SIZE+y] = map[x*SIM_SIZE+y+CHUNK_SIZE];
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
			map[x*SIM_SIZE+y+CHUNK_SIZE] = map[x*SIM_SIZE+y];
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
	memcpy(map,map+SIM_SIZE*CHUNK_SIZE,SIM_SIZE*CHUNK_SIZE);
	memcpy(map+SIM_SIZE*CHUNK_SIZE,map+SIM_SIZE*CHUNK_SIZE*2,SIM_SIZE*CHUNK_SIZE);
	current_chunk.y++;
	for(i4 x = -1;x <= 1;x++) loadChunk((IVEC2){x,1});
}

void worldLoadSouth(){
	for(i4 i = current_chunk.x-1;i <= current_chunk.x+1;i++){
		storeChunk((IVEC2){i,current_chunk.y+1});
	}
	moveEntities(CHUNK_SIZE,VEC2_Y);
	memcpy(map+SIM_SIZE*CHUNK_SIZE*2,map+SIM_SIZE*CHUNK_SIZE,SIM_SIZE*CHUNK_SIZE);
	memcpy(map+SIM_SIZE*CHUNK_SIZE,map,SIM_SIZE*CHUNK_SIZE);
	current_chunk.y--;
	for(i4 x = -1;x <= 1;x++) loadChunk((IVEC2){x,-1});
}