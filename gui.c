#include <stdio.h>

#include "gui.h"
#include "player.h"
#include "source.h"
#include "draw.h"
#include "entity_light.h"
#include "inventory.h"
#include "entity_block.h"
#include "construction.h"
#include "crafting.h"

STRING console_input;

void GUIframe(VEC2 pos,VEC2 size){
	drawRect((VEC2){pos.x,pos.y+size.y},(VEC2){size.x+0.003f,0.005f},COLOR_GREY);
	drawRect((VEC2){pos.x,pos.y-size.y},(VEC2){size.x+0.003f,0.005f},COLOR_GREY);
	drawRect((VEC2){pos.x+size.x,pos.y},(VEC2){0.003f,size.y+0.005f},COLOR_GREY);
	drawRect((VEC2){pos.x-size.x,pos.y},(VEC2){0.003f,size.y+0.005f},COLOR_GREY);
}

void GUIlootBoxEdge(VEC2 pos){
	drawRect((VEC2){pos.x+0.36f,pos.y+0.04f},(VEC2){0.2f,0.005f},COLOR_GREY);
	drawRect((VEC2){pos.x+0.36f,pos.y-0.04f},(VEC2){0.2f,0.005f},COLOR_GREY);
	drawRect((VEC2){pos.x+0.158f,pos.y},(VEC2){0.003f,0.045f},COLOR_GREY);
	drawRect((VEC2){pos.x+0.563f,pos.y},(VEC2){0.003f,0.045f},COLOR_GREY);
}

void GUIcraftingRecipe(VEC2 pos){
	GUIframe((VEC2){pos.x+0.3f,pos.y},RD_GUI(8.0f));
	GUIframe((VEC2){pos.x+0.4f,pos.y},RD_GUI(8.0f));
	GUIframe((VEC2){pos.x+0.5f,pos.y},RD_GUI(8.0f));
	GUIframe((VEC2){pos.x+0.6f,pos.y},RD_GUI(8.0f));
}

void GUIcraftingRecipeItem(VEC2 pos,u1* item){
	u1 item_exist[16];
	for(i4 i = 0;;i++){
		if(!item[i]){
			for(--i;i >= 0;i--){
				VEC2 t_pos = TEXTURE16_RENDER(ITEM_SPRITE_OFFSET+item[i]);
				if(item_exist[i]){
					drawEnemy((VEC2){pos.x+0.3f+0.1f*i,pos.y},RD_GUI(6.0f),t_pos,COLOR_WHITE);
					inventory.item_ammount[item[i]]++;
				}
				else drawEnemy((VEC2){pos.x+0.3f+0.1f*i,pos.y},RD_GUI(6.0f),t_pos,COLOR_GREY);
			}
			break;
		}
		item_exist[i] = inventory.item_ammount[item[i]] ? inventory.item_ammount[item[i]]-- : 0;
	}
}

void GUIdrawInventorySlots(){
	for(u4 i = 0;i < 9;i++){
		VEC2 draw_pos = VEC2addVEC2R(VEC2mulVEC2R((VEC2){i/3,i%3},GUI_INVENTORY_SLOT_OFFSET),GUI_INVENTORY);
		drawRect(VEC2addVEC2R(draw_pos,(VEC2){0.0f,0.075f}),(VEC2){0.03f,0.005f},(VEC3){0.5f,0.5f,0.5f});
		drawRect(VEC2addVEC2R(draw_pos,(VEC2){0.0f,-0.075f}),(VEC2){0.03f,0.005f},(VEC3){0.5f,0.5f,0.5f});
		drawRect(VEC2addVEC2R(draw_pos,(VEC2){0.075f*RD_CMP,0.0f}),(VEC2){0.003f,0.05f},(VEC3){0.5f,0.5f,0.5f});
		drawRect(VEC2addVEC2R(draw_pos,(VEC2){-0.075f*RD_CMP,0.0f}),(VEC2){0.003f,0.05f},(VEC3){0.5f,0.5f,0.5f});
	}
	drawRect(VEC2addVEC2R(GUI_PRIMARY,(VEC2){0.0f,0.075f}),(VEC2){0.03f,0.005f},GUI_PRIMARY_COLOR);
	drawRect(VEC2addVEC2R(GUI_PRIMARY,(VEC2){0.0f,-0.075f}),(VEC2){0.03f,0.005f},GUI_PRIMARY_COLOR);
	drawRect(VEC2addVEC2R(GUI_PRIMARY,(VEC2){0.075f*RD_CMP,0.0f}),(VEC2){0.003f,0.05f},GUI_PRIMARY_COLOR);
	drawRect(VEC2addVEC2R(GUI_PRIMARY,(VEC2){-0.075f*RD_CMP,0.0f}),(VEC2){0.003f,0.05f},GUI_PRIMARY_COLOR);

	drawRect(VEC2addVEC2R(GUI_SECUNDARY,(VEC2){0.0f,0.075f}),(VEC2){0.03f,0.005f},GUI_SECUNDARY_COLOR);
	drawRect(VEC2addVEC2R(GUI_SECUNDARY,(VEC2){0.0f,-0.075f}),(VEC2){0.03f,0.005f},GUI_SECUNDARY_COLOR);
	drawRect(VEC2addVEC2R(GUI_SECUNDARY,(VEC2){0.075f*RD_CMP,0.0f}),(VEC2){0.003f,0.05f},GUI_SECUNDARY_COLOR);
	drawRect(VEC2addVEC2R(GUI_SECUNDARY,(VEC2){-0.075f*RD_CMP,0.0f}),(VEC2){0.003f,0.05f},GUI_SECUNDARY_COLOR);

	f4 energy = (f4)player.energy/5.0f/ENERGY_MAX;
	drawRect((VEC2){GUI_ENERGY.x+0.16f+energy,GUI_ENERGY.y},(VEC2){energy,0.04f},LOOT_ENERGY_COLOR);
	GUIlootBoxEdge(GUI_ENERGY);

	f4 health = (f4)player.health/5.0f/HEALTH_MAX;
	drawRect((VEC2){GUI_HEALTH.x+0.16f+health,GUI_HEALTH.y},(VEC2){health,0.04f},LOOT_HEALTH_COLOR);
	GUIlootBoxEdge(GUI_HEALTH);

	f4 scrap = (f4)player.scrap/5.0f/SCRAP_MAX;
	drawRect((VEC2){GUI_SCRAP.x+0.16f+scrap,GUI_SCRAP.y},(VEC2){scrap,0.04f},LOOT_SCRAP_COLOR);
	GUIlootBoxEdge(GUI_SCRAP);

	GUIframe(GUI_CRAFTING,GUI_BUTTON_SIZE);
	GUIframe(GUI_SETTINGS,GUI_BUTTON_SIZE);
}

void cursorDraw(VEC2 cursor){
	glUseProgram(sprite_shader);
 	drawSprite(cursor,RD_GUI(2.5f),TEXTURE16_RENDER(SPRITE_CROSSHAIR));
	if(inventory.cursor.item.type){
		drawSprite(cursor,RD_GUI(8.0f),texture16Render(inventory.cursor.item.type+ITEM_SPRITE_OFFSET));
	}
}

u1 cursorInGui(f4 cursor_x){
	return cursor_x > GUI_BEGIN_X ? TRUE : FALSE;
}

void GUIcraftFrame(){
	GUIframe(VEC2_ZERO,GUI_MENU_CRAFT_SIZE);
	drawRect(VEC2_ZERO,GUI_MENU_CRAFT_SIZE,VEC3_ZERO);
	GUIframe(GUI_CRAFT1,GUI_BIGBUTTON_SIZE);
	GUIframe(GUI_CRAFT2,GUI_BIGBUTTON_SIZE);
	GUIframe(GUI_CRAFT3,GUI_BIGBUTTON_SIZE);
	GUIframe(GUI_CRAFT4,GUI_BIGBUTTON_SIZE);
	GUIcraftingRecipe(GUI_CRAFT1);
	GUIcraftingRecipe(GUI_CRAFT2);
	GUIcraftingRecipe(GUI_CRAFT3);
	GUIcraftingRecipe(GUI_CRAFT4);
}

void GUIdraw(){
	for(u4 i = 0;i < INVENTORY_SLOT_AMM;i++){
		ITEM item = inventory.item_all[i].item;
		if(inventory.stackable[item.type] && item.ammount != 1){
			u1 amm[4] = {[3] = 0};
			VEC2 d_pos = getInventoryPos(i);
			sprintf(&amm,"%i",item.ammount);
			drawString(VEC2subVEC2R(d_pos,(VEC2){0.02f,-0.03f}),RD_GUI(2.0f),amm);
		}
	}
	drawString((VEC2){GUI_CRAFTING.x-0.07f,GUI_CRAFTING.y},RD_GUI(2.5f),"crafting");
	drawString((VEC2){GUI_SETTINGS.x-0.07f,GUI_SETTINGS.y},RD_GUI(2.5f),"settings");
	drawString(GUI_ENERGY,RD_GUI(2.5f),"energy=");
	drawString(GUI_HEALTH,RD_GUI(2.5f),"health=");
	drawString(GUI_SCRAP,RD_GUI(2.5f),"scrap =");
	u1 str[80];
	sprintf(str,"%i/%i",player.energy,ENERGY_MAX);
	drawString((VEC2){GUI_ENERGY.x+0.18f,GUI_ENERGY.y},RD_GUI(2.5f),str);
	sprintf(str,"%i/%i",player.health,HEALTH_MAX);
	drawString((VEC2){GUI_HEALTH.x+0.18f,GUI_HEALTH.y},RD_GUI(2.5f),str);
	sprintf(str,"%i/%i",player.scrap,SCRAP_MAX);
	drawString((VEC2){GUI_SCRAP.x+0.18f,GUI_SCRAP.y},RD_GUI(2.5f),str);
	VEC2 cursor = getCursorPosGUI();
	switch(menu_select){
	case MENU_BUILDING_OFFSET+CONSTRUCTION_CHEST:
		glUseProgram(color_shader);
		GUIframe(VEC2_ZERO,GUI_DEBUG_SIZE);
		drawRect(VEC2_ZERO,GUI_DEBUG_SIZE,VEC3_ZERO);
		break;
	case MENU_BUILDING_OFFSET+CONSTRUCTION_BLOCKSTATION:
		glUseProgram(color_shader);
		GUIcraftFrame();
		glUseProgram(entity_dark_shader);
		GUIcraftingRecipeItem(GUI_CRAFT1,RECIPY_STONEWALL);
		GUIcraftingRecipeItem(GUI_CRAFT2,RECIPY_LAMP);
		glUseProgram(font_shader);
		drawString((VEC2){GUI_CRAFT1.x-0.12f,GUI_CRAFT1.y},RD_GUI(2.5f),"stonewall");
		drawString((VEC2){GUI_CRAFT2.x-0.03f,GUI_CRAFT2.y},RD_GUI(2.5f),"lamp");
		cursorDraw(cursor);
		break;
	case MENU_BUILDING_OFFSET+CONSTRUCTION_CRAFTINGSTATION:
		glUseProgram(color_shader);
		GUIcraftFrame();
		glUseProgram(entity_dark_shader);
		GUIcraftingRecipeItem(GUI_CRAFT1,RECIPY_BLOCKSTATION);
		GUIcraftingRecipeItem(GUI_CRAFT2,RECIPY_CHEST);
		glUseProgram(font_shader);
		drawString((VEC2){GUI_CRAFT1.x-0.12f,GUI_CRAFT1.y},RD_GUI(2.5f),"block-station");
		drawString((VEC2){GUI_CRAFT2.x-0.07f,GUI_CRAFT2.y},RD_GUI(2.5f),"chest");
		cursorDraw(cursor);
		break;
	case MENU_DEBUG:
		glUseProgram(color_shader);
		GUIframe(VEC2_ZERO,GUI_DEBUG_SIZE);
		drawRect(VEC2_ZERO,GUI_DEBUG_SIZE,VEC3_ZERO);
		drawRect((VEC2){-GUI_DEBUG_SIZE.x+0.03f+0.02f*console_input.cnt,-0.065f},(VEC2){0.008f,0.0075f},(VEC3){1.0f,1.0f,1.0f});
		glUseProgram(font_shader);
		drawString((VEC2){-GUI_DEBUG_SIZE.x+0.03f,0.05f},RD_GUI(2.5f),"debug console");
		drawString((VEC2){-GUI_DEBUG_SIZE.x+0.03f,-0.05f},RD_GUI(2.5f),console_input.data);
		break;
	case MENU_GAME:
		for(u4 i = 0;i < entity_block.cnt;i++){
			if(map.type[coordToMap(entity_block.state[i].pos.x,entity_block.state[i].pos.y)]==BLOCK_BUILDING_ENTITY){
				VEC2 b_pos = (VEC2){(f4)entity_block.state[i].pos.x+0.5f,(f4)entity_block.state[i].pos.y+0.5f};
				if(VEC2distance(player.pos,b_pos)<BLOCKCRAFT_RANGE){
					drawString(mapCrdToRenderCrd(b_pos),RD_GUI(2.5f),"E");
				}
			}
		}
		cursorDraw(cursor);
		break;
	case MENU_CONSTRUCT:
		if(cursorInGui(cursor.x)) cursorDraw(cursor);
		else{
			cursor = VEC2addVEC2R(getCursorPosMap(),player.pos);
			VEC2sub(&cursor,camera.zoom/2.0f);
			cursor.x = (u4)cursor.x;
			cursor.y = (u4)cursor.y;
			if(construction.size&1) VEC2add(&cursor,0.5f);
			glUseProgram(color_shader);
			GUIframe(mapCrdToRenderCrd(cursor),RD_SQUARE(construction.size));
			glUseProgram(sprite_shader);
		}
		break;
	case MENU_CRAFTING_SIMPLE:
		glUseProgram(color_shader);
		GUIcraftFrame();
		glUseProgram(entity_dark_shader);
		GUIcraftingRecipeItem(GUI_CRAFT1,RECIPY_CRAFTINGSTATION);
		GUIcraftingRecipeItem(GUI_CRAFT2,RECIPY_TORCH);
		glUseProgram(font_shader);
		drawString((VEC2){GUI_CRAFT1.x-0.12f,GUI_CRAFT1.y},RD_GUI(2.5f),"craft-station");
		drawString((VEC2){GUI_CRAFT2.x-0.03f,GUI_CRAFT2.y},RD_GUI(2.5f),"torch");
		cursorDraw(cursor);
		break;
	case MENU_SETTING:
		glUseProgram(color_shader);
		GUIframe(VEC2_ZERO,GUI_MENU_SETTING_SIZE);
		drawRect(VEC2_ZERO,GUI_MENU_SETTING_SIZE,VEC3_ZERO);
		GUIframe(GUI_QUIT,GUI_BIGBUTTON_SIZE);
		GUIframe(GUI_FULLSCREEN,GUI_BIGBUTTON_SIZE);
		glUseProgram(font_shader);
		drawString((VEC2){GUI_QUIT.x-0.03f,GUI_QUIT.y},RD_GUI(2.5f),"quit");
		drawString((VEC2){GUI_FULLSCREEN.x-0.09f,GUI_FULLSCREEN.y},RD_GUI(2.5f),"fullscreen");
		cursorDraw(cursor);
		break;
	}
}