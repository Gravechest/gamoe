#pragma once

#include "small_types.h"
#include "entity_item.h"

#define INVENTORY_SLOT_AMM 11

#define INVENTORYSLOT(ITEM) \
(inventory.stackable[ITEM] ? inventorySlotStackable(ITEM) : inventoryEmptySlot())

enum{
	INVENTORY_PRIMARY,
	INVENTORY_SECUNDARY,
	INVENTORY_INVENTORY
};

enum{
	ITEM_NOTHING,
	ITEM_MELEE,
	ITEM_PICKAXE,
	ITEM_BOMB,
	ITEM_LASER,
	ITEM_STONEDUST,
	ITEM_LOG,
	ITEM_TORCH
};

typedef struct{
	union{
		ITEM;
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
	u2 item_ammount[255];
	u2 stackable[255];
}INVENTORY;

extern INVENTORY inventory;

i4 inventorySearch(u4 item);
void inventoryRemove(u4 item);
i4 inventoryAdd(u4 item);
u4 itemDegrade(u4 slot,u4 ammount);
void inventoryRemoveM(u1* item);
i4 inventoryAddStackable(u4 item);
i4 inventorySlotStackable(u4 item);
i4 inventoryAddItem(u4 item);