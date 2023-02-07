#include <heapapi.h>

#include "construction.h"
#include "source.h"
#include "opengl.h"
#include "entity_block.h"
#include "inventory.h"
#include "chunk.h"
#include "crafting.h"

CONSTRUCTION construction;

void Construction(u4 type,u4 size){
	menu_select = MENU_CONSTRUCT;
	construction.type = type;
	construction.size = size;
}

void constructStonewall(IVEC2 map_crd){
	u4 t_map_loc = coordToMap(map_crd.x,map_crd.y);
	map.type[t_map_loc] = BLOCK_NORMAL;
	map.data[t_map_loc].health = 0xff;
	gl_queue.message[gl_queue.cnt].pos = (IVEC2){map_crd.x,map_crd.y};
	gl_queue.message[gl_queue.cnt++].id = GLMESSAGE_WHOLE_MAPEDIT;
	inventoryRemoveM(RECIPY_STONEWALL);
}

void tileConstruct(IVEC2 map_crd){
	u4 t_map_loc = coordToMap(map_crd.x,map_crd.y);
	for(u4 x = map_crd.x;x < map_crd.x + construction.size;x++){
		for(u4 y = map_crd.y;y < map_crd.y + construction.size;y++){
			if(map.type[coordToMap(x,y)] != BLOCK_AIR) return;
		}
	}
	for(u4 x = map_crd.x;x < map_crd.x + construction.size;x++){
		for(u4 y = map_crd.y;y < map_crd.y + construction.size;y++){
			map.type[coordToMap(x,y)] = BLOCK_BUILDING;
			map.data[coordToMap(x,y)].sub_type = construction.type;
		}
	}
	map.type[coordToMap(map_crd.x,map_crd.y)] = BLOCK_BUILDING_ENTITY;
	u4 t_tile_loc = coordToTileTexture(map_crd.y,map_crd.x);
	u4 building_offset;
	if(construction.size==1) building_offset = (construction.type-64)*4+BUILDING_TEXTURE_SIZE*16;
	else                     building_offset = construction.type*8;

	for(u4 x = 0;x < TILE_TEXTURE_SIZE*construction.size;x++){
		for(u4 y = 0;y < TILE_TEXTURE_SIZE*construction.size;y++){
			tile_texture_data[x*SIM_SIZE*TILE_TEXTURE_SIZE+y+t_tile_loc] = building_texture[building_offset+x*BUILDING_TEXTURE_SIZE+y];
		}
	}
	gl_queue.message[gl_queue.cnt++].id = GLMESSAGE_WHOLE_TILEEDIT;

	switch(construction.type){
	case CONSTRUCTION_CHEST:{
		inventoryRemoveM(RECIPY_CHEST);
		entity_block_global.state[entity_block_global.cnt].pos = map_crd;
		void* mem = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(SLOT)*CHEST_SLOT_AMMOUNT);
		entity_block_global.state[entity_block_global.cnt++].slot = mem;
		break;
	}
	case CONSTRUCTION_LAMP:
		inventoryRemoveM(RECIPY_LAMP);
		break;
	case CONSTRUCTION_BLOCKSTATION:
		inventoryRemoveM(RECIPY_BLOCKSTATION);
		break;
	case CONSTRUCTION_CRAFTINGSTATION:
		inventoryRemoveM(RECIPY_CRAFTINGSTATION);
		break;
	}
	entity_block.state[entity_block.cnt++].pos = map_crd;
	menu_select = MENU_GAME;
}