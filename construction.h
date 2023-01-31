#pragma once

#include "small_types.h"

enum{
	CONSTRUCTION_CRAFTINGSTATION,
	CONSTRUCTION_BLOCKSTATION,
	CONSTRUCTION_STONEWALL
};

typedef struct{
	u1 type;
	u1 size;
}CONSTRUCTION;

extern CONSTRUCTION construction;

void Construction(u4 type,u4 size);