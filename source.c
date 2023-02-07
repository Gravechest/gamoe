#include <Windows.h>
#include <intrin.h>
#include <stdio.h>
#include <math.h>

#include "opengl.h"
#include "source.h"
#include "chunk.h"
#include "tmath.h"
#include "ray.h"
#include "entity_light.h"
#include "player.h"
#include "entity_item.h"
#include "entity_togui.h"
#include "entity_dark.h"
#include "inventory.h"
#include "gui.h"
#include "construction.h"
#include "entity_block.h"

i4 proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR), 1,
PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
24,0, 0, 0, 0, 0, 0,0,0,0,
0,0,0,0,32,0,0,PFD_MAIN_PLANE,
0,0,0,0	};

HINSTANCE hinstance;
WNDCLASS wndclass = {.lpfnWndProc = proc,.lpszClassName = "class",.lpszMenuName = "class"};
HWND window;
HDC dc;
MSG Msg;
IVEC2 client_resolution;

VEC3* vramf;
RGB*  vram;
MAP map;

CAMERA camera = {CHUNK_SIZE,CHUNK_SIZE,CHUNK_SIZE};
CAMERA camera_new = {CHUNK_SIZE,CHUNK_SIZE,CHUNK_SIZE};
PLAYER player = {.pos = PLAYER_SPAWN,.energy = ENERGY_MAX,.health = HEALTH_MAX};

LASERHUB  laser;
RGB* texture16;
RGB* building_texture;
KEYS key_pressed;

u1 fullscreen = TRUE;
u1 menu_select;

RGB* tile_texture_data;

u4 coordToMap(u4 x,u4 y){
	return y*SIM_SIZE+x;
}

u1* loadFile(u1* name){
	HANDLE h = CreateFileA(name,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	u4 fsize = GetFileSize(h,0);
	u1* r = HeapAlloc(GetProcessHeap(),0,fsize+1);
	ReadFile(h,r,fsize,0,0);
	r[fsize] = 0;
	CloseHandle(h);
	return r;
}

RGB* loadBMP(u1* name){
	HANDLE h = CreateFileA(name,GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	u4 fsize = GetFileSize(h,0);
	u1* text = HeapAlloc(GetProcessHeap(),8,fsize+1);
	u4 offset;
	SetFilePointer(h,0x0a,0,0);
	ReadFile(h,&offset,4,0,0);
	SetFilePointer(h,offset,0,0);
	ReadFile(h,text,fsize-offset,0,0);
	CloseHandle(h);
	return text;
}

VEC2 getCursorPosMap(){
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(window,&cursor);
	cursor.y = client_resolution.x-cursor.y;
	return (VEC2){(f4)cursor.x*camera.zoom/client_resolution.x,(f4)cursor.y*camera.zoom/client_resolution.x};
}

VEC2 getCursorPosGUI(){
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(window,&cursor);
	cursor.y = client_resolution.x-cursor.y;
	return (VEC2){(f4)cursor.x/client_resolution.y*2.0f-1.0f,(f4)cursor.y/client_resolution.x*2.0f-1.0f};
}

VEC2 entityPull(VEC2 entity,VEC2 destination,f4 power){
	VEC2 rel_pos = VEC2subVEC2R(destination,entity);
	f4 distance = VEC2length(rel_pos);
	VEC2 to_destination = VEC2divR(rel_pos,distance);
	return VEC2mulR(to_destination,power/(distance*distance));
}

u1 lineOfSight(VEC2 pos_1,VEC2 pos_2){
	VEC2 dir = VEC2subVEC2R(pos_2,pos_1);
	RAY2D ray = ray2dCreate(pos_1,dir);
	while(ray.square_pos.x != (u4)pos_2.x && ray.square_pos.y != (u4)pos_2.y){
		if(map.type[coordToMap(ray.square_pos.x,ray.square_pos.y)]==BLOCK_NORMAL) return FALSE;
		ray2dIterate(&ray);
	}
	return TRUE;
}

void collision(VEC2* pos,VEC2 vel,f4 size){
	f4 inc = size;
	f4 offset;
	while(inc > 1.0f) inc *= 0.5f;
	offset = vel.x < 0.0f ? -size : size;
	for(f4 i = pos->y - size;i <= pos->y + size;i+=inc){
		switch(map.type[coordToMap(pos->x+offset,i)]){
		case BLOCK_AIR: break;
		default:
			pos->x -= vel.x;
			vel.x = 0.0f;
			break;
		}
	}
	offset = vel.y < 0.0f ? -size : size;
	for(f4 i = pos->x - size;i <= pos->x + size;i+=inc){
		switch(map.type[coordToMap(i,pos->y+offset)]){
		case BLOCK_AIR: break;
		default:
			pos->y -= vel.y;
			vel.y = 0.0f;
			break;
		}
	}
}

u1 AABBcollision(VEC2 pos1,VEC2 pos2,f4 size1,f4 size2){
	if(pos1.x < pos2.x + size2 && pos1.x + size1 > pos2.x && 
	pos1.y < pos2.y + size2 && pos1.y + size1 > pos2.y){
		return TRUE;
	}
	return FALSE;
}

u1 pointAABBcollision(VEC2 point,VEC2 aabb,VEC2 size){
	if(point.x < aabb.x + size.x && point.x > aabb.x - size.x && 
	point.y < aabb.y + size.y && point.y > aabb.y - size.y){
		return TRUE;
	}
	return FALSE;
}

void tileConstruct(IVEC2 map_crd){
	u4 t_map_loc = coordToMap(map_crd.x,map_crd.y);
	if(map.type[t_map_loc] != BLOCK_AIR || map.type[t_map_loc+1] != BLOCK_AIR ||
	map.type[t_map_loc+SIM_SIZE] != BLOCK_AIR || map.type[t_map_loc+SIM_SIZE+1] != BLOCK_AIR) return;

	u4 t_tile_loc = coordToTileTexture(map_crd.y,map_crd.x);
	map.type[t_map_loc] = BLOCK_BUILDING_ENTITY;
	map.type[t_map_loc+1] = BLOCK_BUILDING;
	map.type[t_map_loc+SIM_SIZE] = BLOCK_BUILDING;
	map.type[t_map_loc+SIM_SIZE+1] = BLOCK_BUILDING;
	map.data[t_map_loc].sub_type = construction.type;
	map.data[t_map_loc+1].sub_type = construction.type;
	map.data[t_map_loc+SIM_SIZE].sub_type = construction.type;
	map.data[t_map_loc+SIM_SIZE+1].sub_type = construction.type;
	u4 building_offset;
	if(construction.size==1) building_offset = construction.type*4+BUILDING_TEXTURE_SIZE*16;
	else                     building_offset = construction.type*8;
	
	for(u4 x = 0;x < 8;x++){
		for(u4 y = 0;y < 8;y++){
			tile_texture_data[x*SIM_SIZE*TILE_TEXTURE_SIZE+y+t_tile_loc] = building_texture[building_offset+x*BUILDING_TEXTURE_SIZE+y];
		}
	}
	gl_queue.message[gl_queue.cnt++].id = GLMESSAGE_WHOLE_TILEEDIT;

	switch(construction.type){
	case CONSTRUCTION_LAMP:
		inventoryRemoveM(RECIPY_LAMP);
		break;
	case CONSTRUCTION_BLOCKSTATION:
		inventoryRemoveM(RECIPY_BLOCKSTATION);
		break;
	case CONSTRUCTION_CRAFTINGSTATION:
		inventoryRemoveM(RECIPY_CRAFTINGSTATION);
		break;
	}
	entity_block.state[entity_block.cnt++].pos = map_crd;
	menu_select = MENU_GAME;
}

void constructStonewall(IVEC2 map_crd){
	u4 t_map_loc = coordToMap(map_crd.x,map_crd.y);
	map.type[t_map_loc] = BLOCK_NORMAL;
	map.data[t_map_loc].health = 0xff;
	gl_queue.message[gl_queue.cnt].pos = (IVEC2){map_crd.x,map_crd.y};
	gl_queue.message[gl_queue.cnt++].id = GLMESSAGE_WHOLE_MAPEDIT;
}

u1 craftButton(VEC2 pos,VEC2 button_pos,u1* item){
	//check if you have the items
	for(i4 i = 0;;i++){
		if(!--inventory.item_count[item[i]]){
			for(;i >= 0;i--) inventory.item_count[item[i]]++;
			return FALSE;
		}
		if(!item[i]){
			for(--i;i >= 0;i--) inventory.item_count[item[i]]++;
			break;
		}
	}

	if(pointAABBcollision(pos,button_pos,GUI_BIGBUTTON_SIZE)){
		return TRUE;
	}
	return FALSE;
}

i4 proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam){
	switch(msg){
	case WM_SIZE:
		client_resolution = (IVEC2){lParam>>16,lParam&0xffff};
		gl_queue.message[gl_queue.cnt].pos = (IVEC2){client_resolution.x,client_resolution.y}; 
		gl_queue.message[gl_queue.cnt++].id = GLMESSAGE_WND_SIZECHANGE;
		break;
	case WM_KEYDOWN:
		switch(menu_select){
		case MENU_DEBUG:
			if((wParam >= '0' && wParam <= '9') || (wParam >= 'A' && wParam <= 'Z') && console_input.cnt < CONSOLE_INPUT_SIZE){
				console_input.data[console_input.cnt++] = wParam;
			}
			switch(wParam){
			case VK_RETURN:
				if(!memcmp("GIVE",console_input.data,4)){
					if(!memcmp("TORCH",console_input.data+5,5)) inventoryAdd(ITEM_TORCH);
					if(!memcmp("LOG"  ,console_input.data+5,3)) inventoryAdd(ITEM_LOG);
					if(!memcmp("STONEDUST",console_input.data+5,9)) inventoryAdd(ITEM_STONEDUST);
					if(!memcmp("BOMB",console_input.data+5,4)) inventoryAdd(ITEM_BOMB);
					if(!memcmp("PICKAXE",console_input.data+5,7)) inventoryAdd(ITEM_PICKAXE);
					if(!memcmp("MELEE",console_input.data+5,5)) inventoryAdd(ITEM_MELEE);
				}
				break;
			case VK_SPACE:
				console_input.data[console_input.cnt++] = '~'+1;
				break;
			case VK_BACK:
				if(console_input.cnt) console_input.data[--console_input.cnt] = '\0';
				break;
			}
			break;
		}
		switch(wParam){
		case VK_OEM_3:
			menu_select = MENU_DEBUG;
			break;
		case VK_E:
			for(u4 i = 0;i < entity_block.cnt;i++){
				u4 m_loc = coordToMap(entity_block.state[i].pos.x,entity_block.state[i].pos.y);
				if(map.type[m_loc]==BLOCK_BUILDING_ENTITY){
					VEC2 b_pos = (VEC2){(f4)entity_block.state[i].pos.x+0.5f,(f4)entity_block.state[i].pos.y+0.5f};
					if(VEC2distance(b_pos,player.pos) < BLOCKCRAFT_RANGE){
						switch(map.data[m_loc].sub_type){
						case CONSTRUCTION_BLOCKSTATION:
							menu_select = MENU_CRAFTING_BLOCK;
							break;
						case CONSTRUCTION_CRAFTINGSTATION:
							menu_select = MENU_CRAFTING_BUILDING;
							break;
						}

					}
				}
			}
			break;
		case VK_ESCAPE:
			switch(menu_select){
			default:
				menu_select = MENU_GAME;
				break;
			}
			break;
		}
		break;
	case WM_MOUSEWHEEL:{
		i2 scroll_ammount = wParam>>16;
		i4 zoom_adjust = scroll_ammount/15;
		camera_new.zoom -= zoom_adjust;
		if(camera_new.zoom>CHUNK_SIZE) camera_new.zoom = CHUNK_SIZE;
		if(camera_new.zoom<8)          camera_new.zoom = 8;
		VEC2sub(&camera_new.pos,zoom_adjust*200.0f);
		break;
	}
	case WM_LBUTTONUP:
		if(inventory.cursor.item.type){
			VEC2 cursor = getCursorPosGUI();
			if(cursor.x < GUI_BEGIN_X){
				itemEntitySpawn(player.pos,VEC2divR(VEC2normalizeR(playerLookDirection()),3.0f),inventory.cursor.item.item);
				inventory.cursor.item.type = ITEM_NOTHING;
			}
			if(pointAABBcollision(cursor,GUI_PRIMARY,RD_GUI(GUI_ITEM_SIZE))){
				if(inventory.item_primary.type) inventory.item_all[inventory.cursor.preslot] = inventory.item_primary;
				inventory.item_primary = inventory.cursor.item;
				inventory.cursor.item.type = ITEM_NOTHING;
				inventory.item_primary.visible = TRUE;
			}
			else if(pointAABBcollision(cursor,GUI_SECUNDARY,RD_GUI(GUI_ITEM_SIZE))){
				if(inventory.item_secundary.type) inventory.item_all[inventory.cursor.preslot] = inventory.item_secundary;
				inventory.item_secundary = inventory.cursor.item;
				inventory.cursor.item.type = ITEM_NOTHING;
				inventory.item_secundary.visible = TRUE;
			}
			for(u4 i = 0;i < 9;i++){
				VEC2 pos = VEC2addVEC2R(VEC2mulVEC2R((VEC2){i/3,i%3},GUI_INVENTORY_SLOT_OFFSET),GUI_INVENTORY);
				if(pointAABBcollision(cursor,pos,RD_GUI(GUI_ITEM_SIZE))){
					if(inventory.item[i].type) inventory.item_all[inventory.cursor.preslot] = inventory.item[i];
					inventory.item[i] = inventory.cursor.item;
					inventory.cursor.item.type = ITEM_NOTHING;
					inventory.item[i].visible = TRUE;
				}
			}
			if(inventory.cursor.item.type){
				inventory.item_all[inventory.cursor.preslot].visible = FALSE;
				inventory.item_all[inventory.cursor.preslot] = inventory.cursor.item;
				if(!inventory.cursor.preslot){
					entityToGuiSpawn(cursor,GUI_PRIMARY,GUI_ITEM_SIZE,inventory.cursor.preslot,inventory.cursor.item.item);
				}
				else{
					entityToGuiSpawn(cursor,getInventoryPos(inventory.cursor.preslot),GUI_ITEM_SIZE,inventory.cursor.preslot,inventory.cursor.item.item);
				}
				inventory.cursor.item.type = ITEM_NOTHING;
			}
		}
		break;
	case WM_RBUTTONDOWN:{
		VEC2 cursor = getCursorPosGUI();
		switch(menu_select){
		case MENU_GAME:
			if(cursor.x < 0.125f) playerAttack(PLAYERATTACK_SECUNDARY);
			break;
		}
		break;
	}
	case WM_LBUTTONDOWN:{
		VEC2 cursor = getCursorPosGUI();
		switch(menu_select){
		case MENU_CRAFTING_BLOCK:
			if (craftButton(cursor,GUI_CRAFT1,RECIPY_STONEWALL)) Construction(CONSTRUCTION_STONEWALL,1);
			if (craftButton(cursor,GUI_CRAFT2,RECIPY_LAMP))      Construction(CONSTRUCTION_LAMP,1);
			break;
		case MENU_CONSTRUCT:
			cursor = VEC2addVEC2R(getCursorPosMap(),camera.pos);
			cursor.x = (u4)cursor.x;
			cursor.y = (u4)cursor.y;
			switch(construction.type){
			case CONSTRUCTION_STONEWALL:
				constructStonewall((IVEC2){cursor.x,cursor.y});
				break;
			default:
				tileConstruct((IVEC2){cursor.x-1,cursor.y-1});
				break;
			}
			break;
		case MENU_CRAFTING_BUILDING:
			if (craftButton(cursor,GUI_CRAFT1,RECIPY_BLOCKSTATION)) Construction(CONSTRUCTION_BLOCKSTATION,2);
			break;
		case MENU_CRAFTING_SIMPLE:
			if (craftButton(cursor,GUI_CRAFT1,RECIPY_CRAFTINGSTATION)) Construction(CONSTRUCTION_CRAFTINGSTATION,2);
			else if(craftButton(cursor,GUI_CRAFT2,RECIPY_TORCH)){
				i4 slot = inventoryEmptySlot();
				if(slot!=-1){
					entityToGuiSpawn(cursor,getInventoryPos(slot),RD_CONVERT(6.0f),slot,(ITEM){ITEM_TORCH,0xff});
				}
			}
			break;
		case MENU_GAME:
			if(cursor.x < 0.125f) playerAttack(PLAYERATTACK_PRIMARY);
			else if(pointAABBcollision(cursor,GUI_SECUNDARY,RD_GUI(GUI_ITEM_SIZE))){
				inventory.cursor.item = inventory.item_secundary;
				inventory.cursor.preslot = INVENTORY_SECUNDARY;
				inventory.item_secundary.type = ITEM_NOTHING;
			}
			else if(pointAABBcollision(cursor,GUI_PRIMARY,RD_GUI(GUI_ITEM_SIZE))){
				inventory.cursor.item = inventory.item_primary;
				inventory.cursor.preslot = INVENTORY_PRIMARY;
				inventory.item_primary.type = ITEM_NOTHING;
			}
			else{
				for(u4 i = 0;i < 9;i++){
					VEC2 pos = VEC2addVEC2R(VEC2mulVEC2R((VEC2){i/3,i%3},GUI_INVENTORY_SLOT_OFFSET),GUI_INVENTORY);
					if(pointAABBcollision(cursor,pos,RD_GUI(GUI_ITEM_SIZE))){
						inventory.cursor.item = inventory.item[i];
						inventory.cursor.preslot = i+1;
						inventory.item[i].type = ITEM_NOTHING;
						break;
					}
				}
			}
			if(pointAABBcollision(cursor,GUI_SETTINGS,GUI_BUTTON_SIZE)) menu_select = MENU_SETTING;
			if(pointAABBcollision(cursor,GUI_CRAFTING,GUI_BUTTON_SIZE)) menu_select = MENU_CRAFTING_SIMPLE;
			break;
		case MENU_SETTING:
			if(pointAABBcollision(cursor,GUI_QUIT,GUI_BIGBUTTON_SIZE)) ExitProcess(0);
			if(pointAABBcollision(cursor,GUI_FULLSCREEN,GUI_BIGBUTTON_SIZE)){
				fullscreen ^= 1;
				if(fullscreen){
					SetWindowLongPtrA(window,GWL_STYLE,WS_VISIBLE|WS_POPUP);
					SetWindowPos(window,0,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),0);
				}
				else{
					SetWindowLongPtrA(window,GWL_STYLE,WS_VISIBLE|WS_MAXIMIZEBOX|WS_MINIMIZEBOX|WS_SYSMENU|WS_CAPTION|WS_SIZEBOX);
					SetWindowPos(window,0,100,100,1000,560,0);
				}
			}
			break;
		}
		break;
	}
	}
	return DefWindowProcA(hwnd,msg,wParam,lParam);
}

void gametick(){
	for(;;){
		if(menu_select != MENU_SETTING){
			key_pressed.w = GetKeyState(VK_W) & 0x80;
			key_pressed.a = GetKeyState(VK_A) & 0x80;
			key_pressed.s = GetKeyState(VK_S) & 0x80;
			key_pressed.d = GetKeyState(VK_D) & 0x80;

			key_pressed.mouse_right = GetKeyState(VK_RBUTTON) & 0x80;

			playerGameTick();
			entityItemTick();
			entityToGuiTick();
			entityLightTick();
			entityDarkTick();

			switch(inventory.item_secundary.type){
			case ITEM_TORCH:
				if(tRnd()<1.03f) itemDegrade(INVENTORY_SECUNDARY,1);
				break;
			}

			camera_new.shake *= 0.95f;
			for(u4 i = 0;i < laser.cnt;i++){
				if(laser.state[i].health>1){
					laser.state[i].pos_org = player.pos;
					laser.state[i].health--;
				}
				else if(!laser.state[i].health){
					ENTITY_REMOVE(laser,i);
				}
			}
		}
		Sleep(15);
	}
}

void render(){
	SetPixelFormat(dc,ChoosePixelFormat(dc,&pfd),&pfd);
	wglMakeCurrent(dc,wglCreateContext(dc));
	openglInit();
	for(;;){
		opengl();
		SwapBuffers(dc);
	}
}

void main(){
	timeBeginPeriod(1);
	ShowCursor(FALSE);
	building_texture = loadBMP("img/tile.bmp");
	for(u4 i = 0;i < BUILDING_TEXTURE_SIZE*BUILDING_TEXTURE_SIZE;i++){
		u1 r = building_texture[i].r;
		building_texture[i].r = building_texture[i].b;
		building_texture[i].b = r;
	}
	console_input.data = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,CONSOLE_INPUT_SIZE);
	vram   = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*CHUNK_SURFACE);
	vramf  = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(VEC3)*CHUNK_SURFACE);
	map.type = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,SIM_AREA);
	map.data = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,SIM_AREA*sizeof(MAPDATA));
	gl_queue.message = HeapAlloc(GetProcessHeap(),0,sizeof(OPENGLMESSAGE)*1024);
	entity_dark.state  = HeapAlloc(GetProcessHeap(),0,sizeof(ENEMY)*1024);
	chunk.state  = HeapAlloc(GetProcessHeap(),0,sizeof(CHUNK)*1024);
	laser.state  = HeapAlloc(GetProcessHeap(),0,sizeof(LASER)*1024);
	entity_item.state = HeapAlloc(GetProcessHeap(),0,sizeof(ITEM_ENTITY)*1024);
	entity_light.state = HeapAlloc(GetProcessHeap(),0,sizeof(PARTICLE)*1024);
	entity_block.state = HeapAlloc(GetProcessHeap(),0,sizeof(BLOCKENTITY)*1024);
	entity_togui.state = HeapAlloc(GetProcessHeap(),0,sizeof(ENTITYTOGUI)*1024);
	tile_texture_data = HeapAlloc(GetProcessHeap(),0,sizeof(RGB)*TILE_TEXTURE_SURFACE*SIM_AREA);
	texture16 = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*TEXTURE16_ROW_SIZE*TEXTURE16_ROW_SIZE);
	wndclass.hInstance = GetModuleHandleA(0);
	RegisterClassA(&wndclass);
	client_resolution = (IVEC2){GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN)};
	window = CreateWindowExA(0,"class","hello",WS_VISIBLE|WS_POPUP,0,0,client_resolution.x,client_resolution.y,0,0,wndclass.hInstance,0);
	dc = GetDC(window);
	worldLoadSpawn();
	CreateThread(0,0,render,0,0,0);
	CreateThread(0,0,gametick,0,0,0);
	while(GetMessageA(&Msg,window,0,0)){
		TranslateMessage(&Msg);
		DispatchMessageA(&Msg);
	}
}

