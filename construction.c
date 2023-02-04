#include "construction.h"
#include "source.h"

CONSTRUCTION construction;

#include <stdio.h>

void Construction(u4 type,u4 size){
	printf("goud\n");
	menu_select = MENU_CONSTRUCT;
	construction.type = type;
	construction.size = size;
}