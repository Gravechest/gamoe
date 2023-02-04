#pragma once

#include "small_types.h"
#include "entity_item.h"

#define INVENTORY_ALL_SIZE 11

enum{
	INVENTORY_PRIMARY,
	INVENTORY_SECUNDARY,
	INVENTORY_INVENTORY
};

typedef struct{
	union{
		struct{
			u1 type;
			u1 durability;
		};
		ITEM item;
	};
	u1 visible;
}SLOT;

typedef struct{
	struct{
		SLOT item;
		u1 preslot;
	}cursor;
	union{
		struct{
			union{
				struct{
					SLOT item_primary;
					SLOT item_secundary;
				};
				SLOT item_attack[2];
			};
			SLOT item[9];
		};
		SLOT item_all[11];
	};
	u1 item_count[255];
}INVENTORY;

extern INVENTORY inventory;

i4 inventorySearch(u4 item);
void inventoryRemove(u4 item);
void inventoryAdd(u4 item);
u4 itemDegrade(u4 slot,u4 ammount);
void inventoryRemoveM(u1* item);