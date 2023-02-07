#include "crafting.h"
#include "gui.h"
#include "source.h"
#include "entity_togui.h"

void craftItem(u4 item,VEC2 cursor){
	i4 slot = INVENTORYSLOT(item);
	if(slot!=-1){
		if(inventory.item_all[slot].type == ITEM_NOTHING){
			inventory.item_all[slot].item = (ITEM){.type = item,.durability = 0xff};
			entityToGuiSpawn(cursor,getInventoryPos(slot),RD_CONVERT(6.0f),slot);
		}
		else{
			inventory.item_all[slot].item.ammount++;
			entityToGuiSpawn(cursor,getInventoryPos(slot),RD_CONVERT(6.0f),slot);
		}
	}
}

u1 craftButton(VEC2 pos,VEC2 button_pos,u1* item){
	if(pointAABBcollision(pos,button_pos,GUI_BIGBUTTON_SIZE)){
		//check if you have the items
		for(i4 i = 0;;i++){
			if(!item[i]){
				for(--i;i >= 0;i--) inventory.item_ammount[item[i]]++;
				break;
			}
			if(!inventory.item_ammount[item[i]]){
				for(--i;i >= 0;i--) inventory.item_ammount[item[i]]++;
				return FALSE;
			}
			inventory.item_ammount[item[i]]--;
		}
		return TRUE;
	}
	return FALSE;
}