#pragma once

#include "small_types.h"

enum{
	//2X2
	CONSTRUCTION_CRAFTINGSTATION,
	CONSTRUCTION_BLOCKSTATION,
	CONSTRUCTION_STONEWALL,
	
	//1X1
	CONSTRUCTION_LAMP = 64
};

typedef struct{
	u1 type;
	u1 size;
}CONSTRUCTION;

extern CONSTRUCTION construction;

void Construction(u4 type,u4 size);