#include "inventory.h"
#include "vec2.h"
#include "gui.h"
#include "source.h"

INVENTORY inventory = {.item_primary = {ITEM_BOMB,30,TRUE},.item_secundary = {ITEM_TORCH,0xff,TRUE},
.stackable[ITEM_LOG] = TRUE,
.stackable[ITEM_STONEDUST] = TRUE,
.stackable[ITEM_BOMB] = TRUE};

u4 itemDegrade(u4 slot,u4 ammount){
	if(inventory.item_all[slot].durability < ammount){
		inventory.item_ammount[inventory.item_all[slot].type]--;
		inventory.item_all[slot].type = ITEM_NOTHING;
	}
	inventory.item_all[slot].durability -= ammount;
}

i4 inventoryEmptySlot(){
	for(i4 j = INVENTORY_SLOT_AMM-1;j >= 0;j--){
		if(!inventory.item_all[j].type){
			return j;
		}
	}
	return -1;
}

i4 inventorySearch(u4 item){
	for(i4 j = INVENTORY_SLOT_AMM-1;j >= 0;j--){
		if(inventory.item_all[j].type == item){
			return j;
		}
	}
	return -1;
}

i4 inventorySlotStackable(u4 item){
	i4 slot = inventorySearch(item);
	if(slot!=-1) return slot;
	return inventoryEmptySlot();
}

void inventoryRemove(u4 item){
	i4 slot = inventorySearch(item);
	inventory.item_ammount[item]--;
	if(inventory.stackable[item]){
		if(!--inventory.item_all[slot].ammount){
			inventory.item_all[slot].type = ITEM_NOTHING;
		}
	}
	else inventory.item_all[slot].type = ITEM_NOTHING;
}

i4 inventoryAdd(u4 item){
	i4 slot = inventoryEmptySlot(item);
	if(slot!=-1){
		inventory.item_all[slot] = (SLOT){.type = item,.durability = 0xff,.visible = TRUE};
		inventory.item_ammount[item]++;
	}
	return slot;
}

i4 inventoryAddStackable(u4 item){
	for(i4 j = INVENTORY_SLOT_AMM-1;j >= 0;j--){
		if(inventory.item_all[j].type == item){
			inventory.item_all[j].ammount++;
			inventory.item_all[j].visible = TRUE;
			inventory.item_all[j].type = item;
			inventory.item_ammount[item]++;
			return j;
		}
	}
	i4 slot = inventoryEmptySlot(item);
	if(slot!=-1){
		inventory.item_all[slot] = (SLOT){.type = item,.ammount = 1,.visible = TRUE};
		inventory.item_ammount[item]++;
	}
	return slot;
}

i4 inventoryAddItem(u4 item){
	if(inventory.stackable[item]) return inventoryAddStackable(item);
	else                          return inventoryAdd(item);
}

void inventoryRemoveM(u1* item){
	while(*item++) inventoryRemove(*item);
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