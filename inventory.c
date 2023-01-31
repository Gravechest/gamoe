#include "inventory.h"
#include "vec2.h"
#include "gui.h"
#include "source.h"

INVENTORY inventory = {.item_primary = {ITEM_BOMB,0xff,TRUE},.item_secundary = {ITEM_TORCH,0xff,TRUE}};

u4 itemDegrade(u4 slot,u4 ammount){
	if(inventory.item_all[slot].durability < ammount){
		inventory.item_count[inventory.item_all[slot].type]--;
		inventory.item_all[slot].type = ITEM_NOTHING;
	}
	inventory.item_all[slot].durability -= ammount;
}

i4 inventoryEmptySlot(){
	for(u4 j = 0;j < INVENTORY_ALL_SIZE;j++){
		if(!inventory.item_all[j].type){
			return j;
		}
	}
	return -1;
}

i4 inventorySearch(u4 item){
	for(u4 j = 0;j < INVENTORY_ALL_SIZE;j++){
		if(inventory.item_all[j].type == item){
			return j;
		}
	}
	return -1;
}

void inventoryRemove(u4 item){
	i4 slot = inventorySearch(item);
	inventory.item_all[slot].type = ITEM_NOTHING;
	inventory.item_count[item]--;
}

void inventoryAdd(u4 item){
	i4 slot = inventoryEmptySlot(item);
	if(slot!=-1){
		inventory.item_all[slot] = (SLOT){.type = item,.durability = 0xff,.visible = TRUE};
		inventory.item_count[item]++;
	}
}

void inventoryCrafting(u4 item1,u4 item2,u4 item3,u4 item4){
	inventory.item_count[item1]--;
	inventory.item_count[item2]--;
	inventory.item_count[item3]--;
	inventory.item_count[item4]--;
	inventory.item_all[inventorySearch(item1)].type = ITEM_NOTHING;
	inventory.item_all[inventorySearch(item2)].type = ITEM_NOTHING;
	inventory.item_all[inventorySearch(item3)].type = ITEM_NOTHING;
	inventory.item_all[inventorySearch(item4)].type = ITEM_NOTHING;
}

VEC2 getInventoryPos(u4 place){
	switch(place){
	case INVENTORY_PRIMARY:   return GUI_PRIMARY;
	case INVENTORY_SECUNDARY: return GUI_SECUNDARY;
	default:{
		place-=2;
		VEC2 position = VEC2mulVEC2R((VEC2){place/3,place%3},GUI_INVENTORY_SLOT_OFFSET);
		return VEC2addVEC2R(position,GUI_INVENTORY);
	}
	}
}