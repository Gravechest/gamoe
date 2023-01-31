#include "entity_block.h"
#include "entity_light.h"
#include "source.h"

BLOCKENTITYHUB entity_block;

void entityBlockTick(){
	for(u4 i = 0;i < entity_block.cnt;i++){
		if(!entity_block.state[i].countdown--){
			if(entity_light.cnt < 128){
				entity_light.state[entity_light.cnt].size = 0.0f;
				entity_light.state[entity_light.cnt].color = VEC3_ZERO;
				entity_light.state[entity_light.cnt].type = LOOT_INFANT;
				f4 r = tRnd();
				if(r<1.8f) entity_light.state[entity_light.cnt].loot_type = LOOT_ENERGY;
				else       entity_light.state[entity_light.cnt].loot_type = LOOT_HEALTH;
				entity_light.state[entity_light.cnt].health = 30;
				entity_light.state[entity_light.cnt++].pos = (VEC2){entity_block.state[i].pos.x+0.5f,entity_block.state[i].pos.y+0.5f};
			}
			entity_block.state[i].countdown = 60*5;
		}
	}
}