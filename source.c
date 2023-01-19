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

VEC3* vramf;
RGB*  vram;
u1* map;

CAMERA camera = {CHUNK_SIZE,CHUNK_SIZE,CHUNK_SIZE};
CAMERA camera_new = {CHUNK_SIZE,CHUNK_SIZE,CHUNK_SIZE};
PLAYER player = {.pos = {CHUNK_SIZE/2+CHUNK_SIZE,CHUNK_SIZE/2+CHUNK_SIZE},.energy = ENERGY_MAX};

BULLETHUB bullet;
ENEMYHUB  enemy;
LASERHUB  laser;
PARTICLEHUB particle;
BLOCKENTITYHUB block_entity;
RGB* texture16;

u1 fullscreen = 1;

void genMap(IVEC2 crd,u4 offset,u4 depth,f4 value){
	if(!depth){
		if(value > 0.0f) map[crd.x*SIM_SIZE+crd.y+offset] = BLOCK_NORMAL;
		return;
	}
	else{
		crd.x *= 2;
		crd.y *= 2;
		genMap((IVEC2){crd.x  ,crd.y  },offset,depth-1,value+tRnd()-1.5f);
		genMap((IVEC2){crd.x+1,crd.y  },offset,depth-1,value+tRnd()-1.5f);
		genMap((IVEC2){crd.x  ,crd.y+1},offset,depth-1,value+tRnd()-1.5f);
		genMap((IVEC2){crd.x+1,crd.y+1},offset,depth-1,value+tRnd()-1.5f);
	}
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
	return VEC2mulR(to_destination,power/distance);
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
	case WM_KEYDOWN:
		switch(wParam){
		case VK_F:
			fullscreen ^= 1;
			if(fullscreen){
				SetWindowLongPtrA(window,GWL_STYLE,WS_VISIBLE|WS_POPUP);
				SetWindowPos(window,0,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),0);
			}
			else{
				SetWindowLongPtrA(window,GWL_STYLE,WS_VISIBLE|WS_MAXIMIZEBOX|WS_MINIMIZEBOX|WS_SYSMENU|WS_CAPTION|WS_SIZEBOX);
				SetWindowPos(window,0,100,100,1000,1000,0);
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
			for(u4 i = 0;i < enemy.cnt;i++){
				if(rayIntersectSquare(player.pos,direction,enemy.state[i].pos,ENEMY_SIZE/2.0f) != -1.0f){
					u4 iterations = (u4)tAbsf(player.pos.x-enemy.state[i].pos.x)+(u4)tAbsf(player.pos.y-enemy.state[i].pos.y);
					f4 dst = VEC2distance(player.pos,enemy.state[i].pos);
					if(lineOfSight(player.pos,direction,iterations) && min_dst > dst){
						min_dst = dst;
						id = i;
					}
				}
			}
			if(id!=-1){
				VEC2 ray_end_pos = VEC2addVEC2R(player.pos,VEC2mulR(VEC2normalizeR(direction),min_dst));
				for(u4 j = 0;j < 3;j++){
					particle.state[particle.cnt].color = VEC3mulR(LASER_LUMINANCE,5.0f);
					particle.state[particle.cnt].vel = VEC2mulR(VEC2rotR(direction,(tRnd()-1.5f)*0.5f),(tRnd()-0.5f)*0.01f);
					particle.state[particle.cnt].health = tRnd()*200.0f;
					particle.state[particle.cnt++].pos = ray_end_pos;
				}
				laser.state[laser.cnt].pos_dst = ray_end_pos;
				for(u4 i = 0;i < enemy.cnt;i++){
					if(id==i) continue;
					VEC2subVEC2(&enemy.state[i].vel,entityPull(enemy.state[i].pos,enemy.state[id].pos,1.0f));
				}
				ENTITY_REMOVE(enemy,id);
			}
			else{
				VEC2 ray_end_pos = ray2dGetCoords(ray);
				laser.state[laser.cnt].pos_dst = ray_end_pos;
			}
		end:
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

void gametick(){
	for(;;){
		u1 key_w = GetKeyState(VK_W) & 0x80;
		u1 key_a = GetKeyState(VK_A) & 0x80;
		u1 key_s = GetKeyState(VK_S) & 0x80;
		u1 key_d = GetKeyState(VK_D) & 0x80;
		if(key_w){
			if(key_d || key_a) player.vel.y+=0.04f * 0.7f;
			else               player.vel.y+=0.04f;
		}
		if(key_s){
			if(key_d || key_a) player.vel.y-=0.04f * 0.7f;
			else               player.vel.y-=0.04f;
		}
		if(key_d){
			if(key_s || key_w) player.vel.x+=0.04f * 0.7f;
			else               player.vel.x+=0.04f;
		}
		if(key_a){
			if(key_s || key_w) player.vel.x-=0.04f * 0.7f;
			else               player.vel.x-=0.04f;
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
		if(/*tRnd()<1.02f && */enemy.cnt < 0){
			enemy.state[enemy.cnt++].pos = (VEC2){tRnd()*CHUNK_SIZE,tRnd()*CHUNK_SIZE};
		}
		for(u4 i = 0;i < enemy.cnt;i++){
			u4 iterations = (u4)fabsf(player.pos.x-enemy.state[i].pos.x)+(u4)fabsf(player.pos.y-enemy.state[i].pos.y);
			VEC2 toPlayer = VEC2normalizeR(VEC2subVEC2R(player.pos,enemy.state[i].pos));
			if(lineOfSight(enemy.state[i].pos,toPlayer,iterations)){
				VEC2div(&toPlayer,50.0f);
				VEC2addVEC2(&enemy.state[i].vel,toPlayer);
			}
			else if(tRnd() < 1.05f){
				VEC2addVEC2(&enemy.state[i].vel,(VEC2){(tRnd()-1.5f)/2.5f,(tRnd()-1.5f)/2.5f});
			}
			VEC2addVEC2(&enemy.state[i].pos,enemy.state[i].vel);
			collision(&enemy.state[i].pos,enemy.state[i].vel,ENEMY_SIZE/2.0f);
			for(u4 j = 0;j < enemy.cnt;j++){
				if(i==j) continue;
				if(enemy.state[i].pos.x<enemy.state[j].pos.x+ENEMY_SIZE&&enemy.state[i].pos.x>enemy.state[j].pos.x-ENEMY_SIZE&&
				enemy.state[i].pos.y<enemy.state[j].pos.y+ENEMY_SIZE&&enemy.state[i].pos.y>enemy.state[j].pos.y-ENEMY_SIZE){
					VEC2subVEC2(&enemy.state[i].vel,VEC2mulR(VEC2normalizeR(VEC2subVEC2R(enemy.state[j].pos,enemy.state[i].pos)),0.01f));
				}
			}
			VEC2mul(&enemy.state[i].vel,PR_FRICTION);	
			if(enemy.state[i].pos.x < ENEMY_SIZE || enemy.state[i].pos.x > SIM_SIZE-ENEMY_SIZE-1.0f ||
			enemy.state[i].pos.y < ENEMY_SIZE || enemy.state[i].pos.y > SIM_SIZE-ENEMY_SIZE-1.0f){
				ENTITY_REMOVE(enemy,i);
			}
		}
		for(u4 i = 0;i < bullet.cnt;i++){
			VEC2addVEC2(&bullet.state[i].pos,bullet.state[i].vel);
			if(bullet.state[i].pos.x < 0.0f || bullet.state[i].pos.x > SIM_SIZE ||
			bullet.state[i].pos.y < 0.0f || bullet.state[i].pos.y > SIM_SIZE ||
			map[(u4)bullet.state[i].pos.y*SIM_SIZE+(u4)bullet.state[i].pos.x] == BLOCK_NORMAL){
				ENTITY_REMOVE(bullet,i);
			}
		}
		for(u4 i = 0;i < laser.cnt;i++){
			if(laser.state[i].health--) VEC2addVEC2(&laser.state[i].pos_org,player.vel);
			else{
				ENTITY_REMOVE(laser,i);
			}
		}
		for(u4 i = 0;i < particle.cnt;i++){
			switch(particle.state[i].type){
			case PARTICLE_NORMAL:
				if(particle.state[i].health--){
					VEC2addVEC2(&particle.state[i].pos,particle.state[i].vel);
					collision(&particle.state[i].pos,particle.state[i].vel,0.5f);
					if(particle.state[i].health<7) VEC3mul(&particle.state[i].color,0.8f);
				}
				else{
					ENTITY_REMOVE(particle,i);
				}
				break;
			case PARTICLE_ENERGY_INFANT:
				if(particle.state[i].health--){
					VEC3addVEC3(&particle.state[i].color,(VEC3){0.001f,0.001f,0.0002f});
					particle.state[i].size += 0.01f;
				}
				else{
					particle.state[i].vel = VEC2rndR();
					particle.state[i].type = PARTICLE_ENERGY_PARENT;
				}
				break;
			case PARTICLE_ENERGY_PARENT:
				VEC2addVEC2(&particle.state[i].pos,particle.state[i].vel);
				collision(&particle.state[i].pos,particle.state[i].vel,particle.state[i].size/2.0f);
				if(VEC2distance(particle.state[i].pos,player.pos)<0.5f){
					player.energy += 1000;
					if(player.energy>ENERGY_MAX) player.energy = ENERGY_MAX;
					ENTITY_REMOVE(particle,i);
					break;
				}	
				VEC2mul(&particle.state[i].vel,PR_FRICTION);
				VEC2addVEC2(&particle.state[i].vel,entityPull(particle.state[i].pos,player.pos,0.1f));
				break;
			}
		}
		for(u4 i = 0;i < block_entity.cnt;i++){
			if(!block_entity.state[i].countdown--){
				particle.state[particle.cnt].size = 0.0f;
				particle.state[particle.cnt].color = (VEC3){0.0f,0.0f,0.0f};
				particle.state[particle.cnt].type = PARTICLE_ENERGY_INFANT;
				particle.state[particle.cnt].health = 60*2;
				particle.state[particle.cnt++].pos = (VEC2){block_entity.state[i].pos.x+0.5f,block_entity.state[i].pos.y+0.5f};
				block_entity.state[i].countdown = 60*15;
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
	bullet.state = HeapAlloc(GetProcessHeap(),0,sizeof(BULLET)*255);
	enemy.state  = HeapAlloc(GetProcessHeap(),0,sizeof(ENEMY)*255);
	chunk.state  = HeapAlloc(GetProcessHeap(),0,sizeof(CHUNK)*255);
	laser.state  = HeapAlloc(GetProcessHeap(),0,sizeof(LASER)*255);
	particle.state     = HeapAlloc(GetProcessHeap(),0,sizeof(PARTICLE)*255);
	block_entity.state = HeapAlloc(GetProcessHeap(),0,sizeof(BLOCKENTITY)*255);
	texture16 = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*TEXTURE16_SIZE*TEXTURE16_SIZE);
	wndclass.hInstance = GetModuleHandleA(0);
	RegisterClassA(&wndclass);
	window = CreateWindowExA(0,"class","hello",WS_VISIBLE | WS_POPUP,WNDOFFX,WNDOFFY,WNDY,WNDX,0,0,wndclass.hInstance,0);
	dc = GetDC(window);
	for(u4 x = 0;x <= CHUNK_SIZE*2;x+=CHUNK_SIZE){
		for(u4 y = 0;y <= SIM_SIZE*CHUNK_SIZE*2;y+=SIM_SIZE*CHUNK_SIZE){
			genMap((IVEC2){0,0},x+y,7,-1.0f);
		}
	}
	//make sure the spawnarea is clean
	for(u4 x = player.pos.x - 3.0f;x < player.pos.x + 3.0f;x++){
		for(u4 y = player.pos.y - 3.0f;y < player.pos.y + 3.0f;y++){
			map[x*SIM_SIZE+y] = 0;
		}
	}
	map[SIM_SIZE*SIM_SIZE/2+SIM_SIZE/2] = BLOCK_SPRINKLER;
	for(u4 i = 0;i < SIM_SIZE_SURFACE;i++){
		switch(map[i]){
		case BLOCK_SPRINKLER:
			block_entity.state[block_entity.cnt].pos = (IVEC2){i/SIM_SIZE,i%SIM_SIZE};
			block_entity.state[block_entity.cnt++].countdown = 0;
			break;
		}
	}
	genTextures();
	CreateThread(0,0,render,0,0,0);
	CreateThread(0,0,gametick,0,0,0);
	while(GetMessageA(&Msg,window,0,0)){
		TranslateMessage(&Msg);
		DispatchMessageA(&Msg);
	}
}

