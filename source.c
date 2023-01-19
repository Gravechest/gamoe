#include <Windows.h>
#include <intrin.h>
#include <stdio.h>
#include <math.h>

#include "opengl.h"
#include "source.h"
#include "chunk.h"
#include "tmath.h"
#include "textures.h"
#include "ray.h"

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
u1* map;

CAMERA camera = {CHUNK_SIZE,CHUNK_SIZE,CHUNK_SIZE};
CAMERA camera_new = {CHUNK_SIZE,CHUNK_SIZE,CHUNK_SIZE};
PLAYER player = {.pos = PLAYER_SPAWN,.energy = ENERGY_MAX,.health = HEALTH_MAX};

ENEMYHUB  entity_dark;
LASERHUB  laser;
PARTICLEHUB entity_light;
BLOCKENTITYHUB entity_block;
RGB* texture16;

u1 fullscreen = 1;

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

VEC2 getCursorPos(){
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(window,&cursor);
	return PLAYER_MOUSE(cursor);
}

VEC2 getCursorPosMap(){
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(window,&cursor);
	cursor.y = WNDX-cursor.y;
	return (VEC2){(f4)cursor.x*camera.zoom/WNDX,(f4)cursor.y*camera.zoom/WNDX};
}

VEC2 entityPull(VEC2 entity,VEC2 destination,f4 power){
	VEC2 rel_pos = VEC2subVEC2R(destination,entity);
	f4 distance = VEC2length(rel_pos);
	VEC2 to_destination = VEC2divR(rel_pos,distance);
	return VEC2mulR(to_destination,power/(distance*distance));
}

u1 lineOfSight(VEC2 pos,VEC2 dir,u4 iterations){
	RAY2D ray = ray2dCreate(pos,dir);
	for(u4 i = 0;i < iterations;i++){
		if(map[(u4)(ray.square_pos.y)*SIM_SIZE+(u4)(ray.square_pos.x)]==BLOCK_NORMAL) return 0;
		ray2dIterate(&ray);
	}
	return 1;
}

i4 proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam){
	switch(msg){
	case WM_SIZE:
		gl_queue.message[gl_queue.cnt].pos = (IVEC2){lParam>>16,lParam&0xffff}; 
		gl_queue.message[gl_queue.cnt++].id = GLMESSAGE_WND_SIZECHANGE;
		break;
	case WM_KEYDOWN:
		switch(wParam){
		case VK_F:
			fullscreen ^= 1;
			if(fullscreen){
				SetWindowLongPtrA(window,GWL_STYLE,WS_VISIBLE|WS_POPUP);
				SetWindowPos(window,0,0,0,client_resolution.x,client_resolution.y,0);
				gl_queue.message[gl_queue.cnt].pos = client_resolution; 
				gl_queue.message[gl_queue.cnt++].id = GLMESSAGE_WND_SIZECHANGE;
			}
			else{
				SetWindowLongPtrA(window,GWL_STYLE,WS_VISIBLE|WS_MAXIMIZEBOX|WS_MINIMIZEBOX|WS_SYSMENU|WS_CAPTION|WS_SIZEBOX);
				SetWindowPos(window,0,100,100,1000,560,0);
				gl_queue.message[gl_queue.cnt].pos = (IVEC2){900,460}; 
				gl_queue.message[gl_queue.cnt++].id = GLMESSAGE_WND_SIZECHANGE;
			}
			break;
		case VK_L:
			map[(u4)player.pos.y*SIM_SIZE+(u4)player.pos.x] = 2;
			gl_queue.message[gl_queue.cnt].id = 0;
			gl_queue.message[gl_queue.cnt++].pos = (IVEC2){player.pos.x,player.pos.y};
			break;
		}
		break;
	case WM_MOUSEWHEEL:{
		i2 scroll_ammount = wParam>>16;
		i4 zoom_adjust = scroll_ammount/30;
		camera_new.zoom -= zoom_adjust;
		if(camera_new.zoom>CHUNK_SIZE) camera_new.zoom = CHUNK_SIZE;
		if(camera_new.zoom<8) camera_new.zoom = 8;
		VEC2sub(&camera_new.pos,zoom_adjust*200.0f);
		break;
	}
	case WM_LBUTTONDOWN:
		if(!player.weapon_cooldown && player.energy > WEAPON_LASER_COST){
			player.energy -= WEAPON_LASER_COST;
			VEC2 direction = VEC2subVEC2R(getCursorPosMap(),VEC2subVEC2R(player.pos,camera.pos));
			RAY2D ray = ray2dCreate(player.pos,direction);
			while(ray.square_pos.x >= 0 && ray.square_pos.x < SIM_SIZE && ray.square_pos.y >= 0 && ray.square_pos.y < SIM_SIZE){
				if(map[ray.square_pos.y*SIM_SIZE+ray.square_pos.x]==BLOCK_NORMAL) break;
				ray2dIterate(&ray);
			}
			f4 min_dst = 99999.0f;
			i4 id = -1;
			for(u4 i = 0;i < entity_dark.cnt;i++){
				if(rayIntersectSquare(entity_dark.state[i].pos,direction,player.pos,ENEMY_SIZE/2.0f) != -1.0f){
					u4 iterations = (u4)tAbsf(player.pos.x-entity_dark.state[i].pos.x)+(u4)tAbsf(player.pos.y-entity_dark.state[i].pos.y);
					f4 dst = VEC2distance(player.pos,entity_dark.state[i].pos);
					if(lineOfSight(player.pos,direction,iterations) && min_dst > dst){
						min_dst = dst;
						id = i;
					}
				}
			}
			if(id!=-1){
				VEC2 ray_end_pos = VEC2addVEC2R(player.pos,VEC2mulR(VEC2normalizeR(direction),min_dst));
				for(u4 j = 0;j < 5;j++){
					entity_light.state[entity_light.cnt].size = 0.5f;
					entity_light.state[entity_light.cnt].type = PARTICLE_NORMAL;
					entity_light.state[entity_light.cnt].color = VEC3mulR(LASER_LUMINANCE,0.5f);
					entity_light.state[entity_light.cnt].vel = VEC2mulR(VEC2rotR(direction,(tRnd()-1.5f)*0.5f),(tRnd()-0.5f)*0.01f);
					entity_light.state[entity_light.cnt].health = 30+tRnd()*30.0f;
					entity_light.state[entity_light.cnt++].pos = ray_end_pos;
				}
				laser.state[laser.cnt].pos_dst = ray_end_pos;
				for(u4 i = 0;i < entity_dark.cnt;i++){
					if(id==i) continue;
					VEC2subVEC2(&entity_dark.state[i].vel,entityPull(entity_dark.state[i].pos,entity_dark.state[id].pos,1.0f));
				}
				ENTITY_REMOVE(entity_dark,id);
			}
			else{
				laser.state[laser.cnt].pos_dst = ray2dGetCoords(ray);
			}
			laser.state[laser.cnt].pos_org = player.pos;
			laser.state[laser.cnt++].health = 5;
			player.weapon_cooldown = PLAYER_WEAPON_COOLDOWN;
		}
		break;
	}
	return DefWindowProcA(hwnd,msg,wParam,lParam);
}

void collision(VEC2* pos,VEC2 vel,f4 size){
	f4 inc = size;
	f4 offset;
	while(inc > 1.0f) inc *= 0.5f;
	offset = vel.x < 0.0f ? -size : size;
	for(f4 i = pos->y - size;i <= pos->y + size;i+=inc){
		if(map[(u4)i*SIM_SIZE+(u4)(pos->x+offset)] == BLOCK_NORMAL){
			pos->x -= vel.x;
			vel.x = 0.0f;
			break;
		}
	}
	offset = vel.y < 0.0f ? -size : size;
	for(f4 i = pos->x - size;i <= pos->x + size;i+=inc){
		if(map[(u4)(pos->y+offset)*SIM_SIZE+(u4)i] == BLOCK_NORMAL){
			pos->y -= vel.y;
			vel.y = 0.0f;
			break;
		}
	}
}

u1 AABBcollision(VEC2 pos1,VEC2 pos2,f4 size1,f4 size2){
	if(pos1.x < pos2.x + size2 && pos1.x + size1 > pos2.x && 
	pos1.y < pos2.y + size2 && pos1.y + size1 > pos2.y){
		return 1;
	}
	return 0;
}

void gametick(){
	for(;;){
		u1 key_w = GetKeyState(VK_W) & 0x80;
		u1 key_a = GetKeyState(VK_A) & 0x80;
		u1 key_s = GetKeyState(VK_S) & 0x80;
		u1 key_d = GetKeyState(VK_D) & 0x80;
		if(key_w){
			if(key_d || key_a) player.vel.y+=PLAYER_SPEED * 0.7f;
			else               player.vel.y+=PLAYER_SPEED;
		}
		if(key_s){
			if(key_d || key_a) player.vel.y-=PLAYER_SPEED * 0.7f;
			else               player.vel.y-=PLAYER_SPEED;
		}
		if(key_d){
			if(key_s || key_w) player.vel.x+=PLAYER_SPEED * 0.7f;
			else               player.vel.x+=PLAYER_SPEED;
		}
		if(key_a){
			if(key_s || key_w) player.vel.x-=PLAYER_SPEED * 0.7f;
			else               player.vel.x-=PLAYER_SPEED;
		}
		if(player.energy && GetKeyState(VK_RBUTTON) & 0x80){
			player.flashlight = 1;
			player.energy--;
		}
		else{
			player.flashlight = 0;
		}
		VEC2addVEC2(&player.pos,player.vel);
		collision(&player.pos,player.vel,PLAYER_SIZE/2.0f);
		VEC2mul(&player.vel,PR_FRICTION);
		if(tRnd()<1.01f && entity_dark.cnt < 32){
			VEC2 spawn = (VEC2){(tRnd()-1.0f)*CHUNK_SIZE,(tRnd()-1.0f)*CHUNK_SIZE};
			if(tRnd()<1.5f) spawn.x += CHUNK_SIZE;
			if(tRnd()<1.5f) spawn.y += CHUNK_SIZE;
			entity_dark.state[entity_dark.cnt].pos = spawn;
			entity_dark.state[entity_dark.cnt++].vel = VEC2_ZERO;
		}
		for(u4 i = 0;i < entity_dark.cnt;i++){
			u4 iterations = (u4)fabsf(player.pos.x-entity_dark.state[i].pos.x)+(u4)fabsf(player.pos.y-entity_dark.state[i].pos.y);
			VEC2 toPlayer = VEC2normalizeR(VEC2subVEC2R(player.pos,entity_dark.state[i].pos));
			if(player.health && lineOfSight(entity_dark.state[i].pos,toPlayer,iterations)){
				VEC2div(&toPlayer,65.0f);
				VEC2addVEC2(&entity_dark.state[i].vel,toPlayer);
			}
			else if(tRnd() < 1.05f){
				VEC2addVEC2(&entity_dark.state[i].vel,(VEC2){(tRnd()-1.5f)/2.5f,(tRnd()-1.5f)/2.5f});
			}
			VEC2addVEC2(&entity_dark.state[i].pos,entity_dark.state[i].vel);
			collision(&entity_dark.state[i].pos,entity_dark.state[i].vel,ENEMY_SIZE/2.0f);
			for(u4 j = 0;j < entity_dark.cnt;j++){
				if(i==j) continue;
				if(AABBcollision(entity_dark.state[i].pos,entity_dark.state[j].pos,ENEMY_SIZE,ENEMY_SIZE)){
					VEC2 pushaway = VEC2mulR(VEC2normalizeR(VEC2subVEC2R(entity_dark.state[j].pos,entity_dark.state[i].pos)),0.1f);
					VEC2subVEC2(&entity_dark.state[i].vel,pushaway);
				}
			}
			if(player.health && AABBcollision(entity_dark.state[i].pos,player.pos,ENEMY_SIZE,PLAYER_SIZE)){
				VEC2 pushaway = VEC2mulR(VEC2normalizeR(VEC2subVEC2R(player.pos,entity_dark.state[i].pos)),0.1f);
				VEC2subVEC2(&entity_dark.state[i].vel,pushaway);
				player.health -= 2;
				if(player.health<0){
					player.health = 0;
					player.respawn_countdown = 120;
				}
				VEC2 rel_pos = VEC2subVEC2R(entity_dark.state[i].pos,player.pos);
				VEC2 velocity = VEC2mulR(rel_pos,(tRnd()-1.0f)*0.1f); 
				VEC2rot(&velocity,(tRnd()-1.5f)*0.5f);
				entity_light.state[entity_light.cnt].color = (VEC3){0.07f,0.02f,0.02f};
				entity_light.state[entity_light.cnt].type = PARTICLE_NORMAL;
				entity_light.state[entity_light.cnt].pos = VEC2addVEC2R(player.pos,rel_pos);
				entity_light.state[entity_light.cnt].size = 1.0f;
				entity_light.state[entity_light.cnt].health = 40;
				entity_light.state[entity_light.cnt++].vel = velocity;
			}
			VEC2mul(&entity_dark.state[i].vel,PR_FRICTION);	
			if(entity_dark.state[i].pos.x < ENEMY_SIZE || entity_dark.state[i].pos.x > SIM_SIZE-ENEMY_SIZE-1.0f ||
			entity_dark.state[i].pos.y < ENEMY_SIZE || entity_dark.state[i].pos.y > SIM_SIZE-ENEMY_SIZE-1.0f){
				ENTITY_REMOVE(entity_dark,i);
			}
		}
		for(u4 i = 0;i < laser.cnt;i++){
			if(laser.state[i].health>1){
				laser.state[i].pos_org = player.pos;
				laser.state[i].health--;
			}
			else if(!laser.state[i].health){
				ENTITY_REMOVE(laser,i);
			}
		}
		for(u4 i = 0;i < entity_light.cnt;i++){
			switch(entity_light.state[i].type){
			case PARTICLE_NORMAL:
				if(entity_light.state[i].health--){
					VEC2addVEC2(&entity_light.state[i].pos,entity_light.state[i].vel);
					collision(&entity_light.state[i].pos,entity_light.state[i].vel,0.5f);
					if(entity_light.state[i].health<15){
						VEC3mul(&entity_light.state[i].color,0.8f);
						entity_light.state[i].size *= 0.8;
					}	
				}
				else{
					ENTITY_REMOVE(entity_light,i);
				}
				break;
			case PARTICLE_ENERGY_INFANT:
				if(entity_light.state[i].health--){
					VEC3addVEC3(&entity_light.state[i].color,(VEC3){0.003f,0.003f,0.0006f});
					entity_light.state[i].size += 0.02f;
				}
				else{
					entity_light.state[i].vel = VEC2rndR();
					entity_light.state[i].type = PARTICLE_ENERGY_PARENT;
					entity_light.state[i].health = 60*60;
				}
				break;
			case PARTICLE_ENERGY_PARENT:
				if(entity_light.state[i].health--){
					VEC2addVEC2(&entity_light.state[i].pos,entity_light.state[i].vel);
					collision(&entity_light.state[i].pos,entity_light.state[i].vel,entity_light.state[i].size/2.0f);
					if(VEC2distance(entity_light.state[i].pos,player.pos)<1.0f){
						player.energy += 200;
						if(player.energy>ENERGY_MAX) player.energy = ENERGY_MAX;
						ENTITY_REMOVE(entity_light,i);
						break;
					}	
					VEC2mul(&entity_light.state[i].vel,PR_FRICTION);
					VEC2addVEC2(&entity_light.state[i].vel,entityPull(entity_light.state[i].pos,player.pos,1.0f));
					if(entity_light.state[i].health<15){
						VEC3mul(&entity_light.state[i].color,0.8f);
						entity_light.state[i].size *= 0.8;
					}	
				}
				else{
					ENTITY_REMOVE(entity_light,i);
				}

				break;
			}
		}
		for(u4 i = 0;i < entity_block.cnt;i++){
			if(!entity_block.state[i].countdown--){
				if(entity_light.cnt < 128){
					entity_light.state[entity_light.cnt].size = 0.0f;
					entity_light.state[entity_light.cnt].color = VEC3_ZERO;
					entity_light.state[entity_light.cnt].type = PARTICLE_ENERGY_INFANT;
					entity_light.state[entity_light.cnt].health = 30;
					entity_light.state[entity_light.cnt++].pos = (VEC2){entity_block.state[i].pos.x+0.5f,entity_block.state[i].pos.y+0.5f};
				}
				entity_block.state[i].countdown = 60*5;
			}
		}
		if(!player.health){
			if(!player.respawn_countdown--){
				player.pos = (VEC2)PLAYER_SPAWN;
				player.health = HEALTH_MAX;
				player.energy = ENERGY_MAX;
				worldLoadSpawn();
				entity_dark.cnt = 0;
				entity_light.cnt = 0;
				laser.cnt = 0;
				chunk.cnt = 0;
				gl_queue.message[gl_queue.cnt++].id = GLMESSAGE_WHOLE_MAPEDIT;
			}
		}
		if(player.weapon_cooldown) player.weapon_cooldown--;
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
	ShowCursor(0);
	vram   = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*CHUNK_SIZE_SURFACE);
	vramf  = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(VEC3)*CHUNK_SIZE_SURFACE);
	map    = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,SIM_SIZE_SURFACE);
	gl_queue.message = HeapAlloc(GetProcessHeap(),0,sizeof(OPENGLMESSAGE)*255);
	entity_dark.state  = HeapAlloc(GetProcessHeap(),0,sizeof(ENEMY)*255);
	chunk.state  = HeapAlloc(GetProcessHeap(),0,sizeof(CHUNK)*255);
	laser.state  = HeapAlloc(GetProcessHeap(),0,sizeof(LASER)*255);
	entity_light.state     = HeapAlloc(GetProcessHeap(),0,sizeof(PARTICLE)*255);
	entity_block.state = HeapAlloc(GetProcessHeap(),0,sizeof(BLOCKENTITY)*255);
	texture16 = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*TEXTURE16_SIZE*TEXTURE16_SIZE);
	wndclass.hInstance = GetModuleHandleA(0);
	RegisterClassA(&wndclass);
	window = CreateWindowExA(0,"class","hello",WS_VISIBLE | WS_POPUP,WNDOFFX,WNDOFFY,WNDY,WNDX,0,0,wndclass.hInstance,0);
	dc = GetDC(window);
	client_resolution = (IVEC2){GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN)};
	worldLoadSpawn();
	genTextures();
	CreateThread(0,0,render,0,0,0);
	CreateThread(0,0,gametick,0,0,0);
	while(GetMessageA(&Msg,window,0,0)){
		TranslateMessage(&Msg);
		DispatchMessageA(&Msg);
	}
}

