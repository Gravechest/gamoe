#include "construction.h"
#include "source.h"

CONSTRUCTION construction;

void Construction(u4 type,u4 size){
	menu_select = MENU_CONSTRUCT;
	construction.type = type;
	construction.size = size;
}