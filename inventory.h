#pragma once

#include "small_types.h"

#define INVENTORY_ALL_SIZE 11

enum{
	INVENTORY_PRIMARY,
	INVENTORY_SECONDARY,
	INVENTORY_INVENTORY
};

typedef struct{
	u1 visible;
	u1 type;
}SLOT;

typedef struct{
	struct{
		u1 item;
		u1 preslot;
	}cursor;
	union{
		struct{
			SLOT item_primary;
			SLOT item_secundary;
			SLOT item[9];
		};
		SLOT item_all[11];
	};
	u1 item_count[255];
}INVENTORY;

extern INVENTORY inventory;

i4 inventorySearch(u4 item);
void inventoryCrafting(u4 item1,u4 item2,u4 item3,u4 item4);