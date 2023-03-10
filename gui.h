#pragma once

#include "vec2.h"
#include "opengl.h"

#define COLOR_WHITE (VEC3){1.0f,1.0f,1.0f}
#define COLOR_GREY (VEC3){0.5f,0.5f,0.5f}

#define GUI_BEGIN_X 0.125f

#define GUI_SECUNDARY_COLOR (VEC3){0.6f,0.6f,0.2f}
#define GUI_PRIMARY_COLOR (VEC3){0.6f,0.2f,0.2f}

#define GUI_ENERGY (VEC2){0.18f,0.9f}
#define GUI_HEALTH (VEC2){0.18f,0.8f}
#define GUI_SCRAP  (VEC2){0.18f,0.7f}
#define GUI_PRIMARY (VEC2){GUI_INVENTORY.x,GUI_INVENTORY.y-0.3f}
#define GUI_SECUNDARY (VEC2){GUI_INVENTORY.x+GUI_INVENTORY_SLOT_OFFSET.x,GUI_INVENTORY.y-0.3f}
#define GUI_INVENTORY (VEC2){0.24f,-0.55f}
#define GUI_BUTTON_SIZE (VEC2){0.1f,0.04f}
#define GUI_BIGBUTTON_SIZE (VEC2){0.2f,0.04f}
#define GUI_SETTINGS (VEC2){0.88f,0.9f}
#define GUI_CRAFTING (VEC2){0.88f,-0.1f}
#define GUI_DEBUG_SIZE (VEC2){0.5f,0.1f}

#define GUI_QUIT (VEC2){0.0f,-0.43f}
#define GUI_FULLSCREEN (VEC2){0.0f,-0.33f}

#define GUI_CRAFT1 (VEC2){-0.27f,0.27f}
#define GUI_CRAFT2 (VEC2){-0.27f,0.09f}
#define GUI_CRAFT3 (VEC2){-0.27f,-0.09f}
#define GUI_CRAFT4 (VEC2){-0.27f,-0.27f}

#define GUI_MENU_CRAFT_SIZE (VEC2){0.5f,0.4f}
#define GUI_MENU_SETTING_SIZE (VEC2){0.3f,0.5f}

#define GUI_ITEM_SIZE 8.0f
#define GUI_INVENTORY_SLOT_OFFSET RD_GUI(32.0f)

#define CONSOLE_INPUT_SIZE 40

typedef struct{
	u4  cnt;
	u1* data;
}STRING;

extern STRING console_input;

void GUIdraw();
void GUIdrawInventorySlots();