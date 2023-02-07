#include "entity_togui.h"
#include "opengl.h"
#include "source.h"
#include "inventory.h"
#include "gui.h"

ENTITYTOGUIHUB entity_togui;

void entityToGuiSpawn(VEC2 pos,VEC2 dst,f4 size,u4 slot){
	inventory.item_all[slot].visible = FALSE;
	entity_togui.state[entity_togui.cnt].pos = pos;
	entity_togui.state[entity_togui.cnt].size = size;
	entity_togui.state[entity_togui.cnt].dst = dst;
	entity_togui.state[entity_togui.cnt].inventory_slot = slot;
	entity_togui.state[entity_togui.cnt++].vel = VEC2_ZERO;
}

void entityToGuiTick(){
	for(u4 i = 0;i < entity_togui.cnt;i++){
		VEC2addVEC2(&entity_togui.state[i].vel,VEC2mulR(VEC2subVEC2R(entity_togui.state[i].dst,entity_togui.state[i].pos),0.002f));
		VEC2addVEC2(&entity_togui.state[i].pos,entity_togui.state[i].vel);
		VEC2mul(&entity_togui.state[i].vel,0.99f);
		VEC2mul(&entity_togui.state[i].size,10.0f);
		VEC2addVEC2(&entity_togui.state[i].size,(VEC2){GUI_ITEM_SIZE,GUI_ITEM_SIZE});
		VEC2div(&entity_togui.state[i].size,11.0f);
		if(VEC2distance(entity_togui.state[i].dst,entity_togui.state[i].pos) < 0.02f){
			inventory.item_all[entity_togui.state[i].inventory_slot].visible = TRUE;
			ENTITY_REMOVE(entity_togui,i);
		}
	}
}