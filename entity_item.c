#include "entity_item.h"
#include "source.h"
#include "player.h"
#include "entity_togui.h"
#include "opengl.h"
#include "inventory.h"

ITEM_ENTITYHUB entity_item;

void blockParticleSpawn(VEC2 pos,VEC2 vel){
	if(pos.x < ENTITY_BLOCK_PARTICLE_SIZE || pos.x > SIM_SIZE-ENTITY_BLOCK_PARTICLE_SIZE-1.0f ||
	pos.y < ENTITY_BLOCK_PARTICLE_SIZE || pos.y > SIM_SIZE-ENTITY_BLOCK_PARTICLE_SIZE-1.0f){
		return;
	}
	entity_item.state[entity_item.cnt].type = ENTITY_BLOCK_PARTICLE;
	entity_item.state[entity_item.cnt].pos = pos;
	entity_item.state[entity_item.cnt].vel = vel;
	entity_item.state[entity_item.cnt++].size = ENTITY_BLOCK_PARTICLE_SIZE;
}

void itemEntitySpawnNew(VEC2 pos,VEC2 vel,u4 item_type){
	entity_item.state[entity_item.cnt].size = ENTITY_ITEM_SIZE;
	entity_item.state[entity_item.cnt].pos = pos;
	entity_item.state[entity_item.cnt].pickup_countdown = 30;
	entity_item.state[entity_item.cnt].vel = vel;
	entity_item.state[entity_item.cnt++].item = (ITEM){item_type,0xff};
}

void itemEntitySpawn(VEC2 pos,VEC2 vel,ITEM item){
	entity_item.state[entity_item.cnt].size = ENTITY_ITEM_SIZE;
	entity_item.state[entity_item.cnt].pos = pos;
	entity_item.state[entity_item.cnt].pickup_countdown = 30;
	entity_item.state[entity_item.cnt].vel = vel;
	entity_item.state[entity_item.cnt++].item = item;
}

void entityItemTick(){
	for(u4 i = 0;i < entity_item.cnt;i++){
		switch(entity_item.state[i].type){
		case ENTITY_BLOCK_PARTICLE:
			VEC2addVEC2(&entity_item.state[i].pos,entity_item.state[i].vel);
			entity_item.state[i].size*=0.9f;
			if(entity_item.state[i].size<0.01f){
				ENTITY_REMOVE(entity_item,i);
				continue;
			}
			break;
		case ENTITY_ITEM:
			VEC2addVEC2(&entity_item.state[i].pos,entity_item.state[i].vel);
			collision(&entity_item.state[i].pos,entity_item.state[i].vel,0.5f);
			VEC2mul(&entity_item.state[i].vel,PR_FRICTION);
			if(!entity_item.state[i].pickup_countdown){
				if(VEC2distance(entity_item.state[i].pos,player.pos)<2.0f){
					i4 slot = inventoryEmptySlot();
					if(slot!=-1){
						VEC2 egui_pos = mapCrdToRenderCrd(entity_item.state[i].pos);
						entityToGuiSpawn(egui_pos,getInventoryPos(slot),ENTITY_ITEM_SIZE,slot,entity_item.state[i].item);
						ENTITY_REMOVE(entity_item,i);
						continue;
					}
				}
			}
			else entity_item.state[i].pickup_countdown--;
			break;
		}
		if(entity_item.state[i].pos.x < entity_item.state[i].size || entity_item.state[i].pos.x > SIM_SIZE-entity_item.state[i].size-1.0f ||
		entity_item.state[i].pos.y < entity_item.state[i].size || entity_item.state[i].pos.y > SIM_SIZE-entity_item.state[i].size-1.0f){
			ENTITY_REMOVE(entity_item,i);
		}
	}
}