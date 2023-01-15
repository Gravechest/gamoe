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
CAMERA camera = {RES,RES,RES};
CAMERA camera_new = {RES,RES,RES};
PLAYER player = {.pos = {RES/2+RES,RES/2+RES}};

BULLETHUB bullet;
ENEMYHUB  enemy;
LASERHUB  laser;
PARTICLEHUB particle;
RGB* texture16;

void genMap(IVEC2 crd,u4 offset,u4 depth,f4 value){
	if(!depth){
		if(value > 0.0f) map[crd.x*MAP+crd.y+offset] = 1;
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

u1 iSquare(VEC2 ro,VEC2 rd,f4 size){
	VEC2sub(&ro,size);
	VEC2 delta = VEC2divFR(rd,1.0f);
	VEC2 n = VEC2mulVEC2R(delta,ro);
	VEC2 k = VEC2mulR(VEC2absR(delta),size);
	VEC2 t1= VEC2subVEC2R(VEC2negR(n),k);
	VEC2 t2= VEC2addVEC2R(VEC2negR(n),k);
	f4 tN = tMaxf(t1.x,t1.y);
	f4 tF = tMinf(t2.x,t2.y);
	if(tN>tF||tF<0.0f) return 0;
	return 1;
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

u1 lineOfSight(VEC2 pos,VEC2 dir,u4 iterations){
	RAY2D ray = ray2dCreate(pos,dir);
	for(u4 i = 0;i < iterations;i++){
		if(map[(u4)(ray.roundPos.y)*MAP+(u4)(ray.roundPos.x)]){
			return 0;	
		}
		ray2dIterate(&ray);
	}
	return 1;
}

i4 proc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam){
	switch(msg){
	case WM_KEYDOWN:
		switch(wParam){
		case VK_L:
			map[(u4)player.pos.y*MAP+(u4)player.pos.x] = 2;
			gl_queue.message[gl_queue.cnt].id = 0;
			gl_queue.message[gl_queue.cnt++].pos = (IVEC2){player.pos.x,player.pos.y};
			break;
		}
		break;
	case WM_MOUSEWHEEL:{
		i2 scroll_ammount = wParam>>16;
		i4 zoom_adjust = scroll_ammount/30;
		camera_new.zoom -= zoom_adjust;
		if(camera_new.zoom>RES) camera_new.zoom = RES;
		if(camera_new.zoom<8)   camera_new.zoom = 8;
		VEC2sub(&camera_new.pos,zoom_adjust*200.0f);
		break;
	}
	case WM_RBUTTONDOWN:
		if(1){
			VEC2 direction = VEC2subVEC2R(getCursorPosMap(),VEC2subVEC2R(player.pos,camera.pos));
			bullet.state[bullet.cnt].pos = player.pos;
			bullet.state[bullet.cnt++].vel = VEC2divR(VEC2normalizeR(direction),5.0f);
			player.lightBulletCnt--;
		}
		break;
	case WM_LBUTTONDOWN:
		if(!player.weapon_cooldown){
			VEC2 direction = VEC2subVEC2R(getCursorPosMap(),VEC2subVEC2R(player.pos,camera.pos));
			RAY2D ray = ray2dCreate(player.pos,direction);
			while(ray.roundPos.x >= 0 && ray.roundPos.x < MAP && ray.roundPos.y >= 0 && ray.roundPos.y < MAP){
				if(map[ray.roundPos.y*MAP+ray.roundPos.x]==1) break;
				ray2dIterate(&ray);
			}
			for(u4 i = 0;i < enemy.cnt;i++){
				if(iSquare(VEC2subVEC2R(player.pos,enemy.state[i].pos),direction,ENEMY_SIZE)){
					u4 iterations = (u4)tAbsf(player.pos.x-enemy.state[i].pos.x)+(u4)tAbsf(player.pos.y-enemy.state[i].pos.y);
					if(lineOfSight(player.pos,direction,iterations)){
						f4 dst = VEC2length(VEC2subVEC2R(player.pos,enemy.state[i].pos));
						VEC2 ray_end_pos = VEC2addVEC2R(player.pos,VEC2mulR(VEC2normalizeR(direction),dst));
						for(u4 j = 0;j < 16;j++){
							particle.state[particle.cnt].color = LASER_LUMINANCE;
							particle.state[particle.cnt].vel = VEC2mulR(VEC2rotR(direction,(tRnd()-1.5f)*0.5f),(tRnd()-0.5f)*0.01f);
							particle.state[particle.cnt].health = tRnd()*20.0f;
							particle.state[particle.cnt++].pos = ray_end_pos;
						}
						laser.state[laser.cnt].pos_dst = ray_end_pos;
						for(u4 j = i;j < enemy.cnt;j++) enemy.state[j] = enemy.state[j+1];
						enemy.cnt--;
						goto end;
					}
				}
			}
			VEC2 ray_end_pos = ray2dGetCoords(ray);
			laser.state[laser.cnt].pos_dst = ray_end_pos;
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
	while(inc > 1.0f) inc *= 0.5f;
	if(vel.x < 0.0f){
		for(f4 i = pos->y - size;i <= pos->y + size;i+=inc){
			if(map[(u4)i*MAP+(u4)(pos->x-size)]){
				pos->x -= vel.x;
				vel.x = 0.0f;
				break;
			}
		}
	}
	else{
		for(f4 i = pos->y - size;i <= pos->y + size;i+=inc){
			if(map[(u4)i*MAP+(u4)(pos->x+size)]){
				pos->x -= vel.x;
				vel.x = 0.0f;
				break;
			}
		}
	}
	if(vel.y < 0.0f){
		for(f4 i = pos->x - size;i <= pos->x + size;i+=inc){
			if(map[(u4)(pos->y-size)*MAP+(u4)i]){
				pos->y -= vel.y;
				vel.y = 0.0f;
				break;
			}
		}
	}
	else{
		for(f4 i = pos->x - size;i <= pos->x + size;i+=inc){
			if(map[(u4)(pos->y+size)*MAP+(u4)i]){
				pos->y -= vel.y;
				vel.y = 0.0f;
				break;
			}
		}
	}
}

void physics(){
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
		VEC2addVEC2(&player.pos,player.vel);
		collision(&player.pos,player.vel,PLAYER_SIZE/2.0f);
		VEC2mul(&player.vel,PR_FRICTION);
		if(tRnd()<1.02f && enemy.cnt < 8){
			enemy.state[enemy.cnt++].pos = (VEC2){tRnd()*RES,tRnd()*RES};
		}
		for(u4 i = 0;i < enemy.cnt;i++){
			if(enemy.state[i].pos.x < 1.0f || enemy.state[i].pos.x > RES*3-1.0f ||
			enemy.state[i].pos.y < 1.0f || enemy.state[i].pos.y > RES*3-1.0f){
				for(u4 j = i;j < enemy.cnt;j++) enemy.state[j] = enemy.state[j+1];
				enemy.cnt--;
				i--;
				continue;
			}
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
			VEC2mul(&enemy.state[i].vel,PR_FRICTION);	
		}
		for(u4 i = 0;i < bullet.cnt;i++){
			VEC2addVEC2(&bullet.state[i].pos,bullet.state[i].vel);
			if(bullet.state[i].pos.x < 0.0f || bullet.state[i].pos.x > MAP ||
			bullet.state[i].pos.y < 0.0f || bullet.state[i].pos.y > MAP ||
			map[(u4)bullet.state[i].pos.y*MAP+(u4)bullet.state[i].pos.x]){
				for(u4 j = i;j < bullet.cnt;j++) bullet.state[j] = bullet.state[j+1];
				bullet.cnt--;
			}
		}
		for(u4 i = 0;i < laser.cnt;i++){
			if(laser.state[i].health--){
				VEC2addVEC2(&laser.state[i].pos_org,player.vel);
			}
			else{
				for(u4 j = i;j < laser.cnt;j++) laser.state[j] = laser.state[j+1];
				laser.cnt--;
			}
		}
		for(u4 i = 0;i < particle.cnt;i++){
			if(particle.state[i].health--){
				VEC2addVEC2(&particle.state[i].pos,particle.state[i].vel);
				collision(&particle.state[i].pos,particle.state[i].vel,0.5f);
				if(particle.state[i].health<7) VEC3mul(&particle.state[i].color,0.8f);
			}
			else{
				for(u4 j = i;j < particle.cnt;j++) particle.state[j] = particle.state[j+1];
				particle.cnt--;
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
	vram   = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*RES*RES);
	vramf  = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(VEC3)*RES*RES);
	map    = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,MAP*MAP);
	gl_queue.message = HeapAlloc(GetProcessHeap(),0,sizeof(OPENGLMESSAGE)*255);
	bullet.state = HeapAlloc(GetProcessHeap(),0,sizeof(BULLET)*255);
	enemy.state  = HeapAlloc(GetProcessHeap(),0,sizeof(ENEMY)*255);
	chunk.state  = HeapAlloc(GetProcessHeap(),0,sizeof(CHUNK)*255);
	laser.state  = HeapAlloc(GetProcessHeap(),0,sizeof(LASER)*255);
	particle.state = HeapAlloc(GetProcessHeap(),0,sizeof(PARTICLE)*255);
	texture16 = HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(RGB)*TEXTURE16_SIZE*TEXTURE16_SIZE);
	wndclass.hInstance = GetModuleHandleA(0);
	RegisterClassA(&wndclass);
	window = CreateWindowExA(0,"class","hello",WS_VISIBLE | WS_POPUP,WNDOFFX,WNDOFFY,WNDY,WNDX,0,0,wndclass.hInstance,0);
	dc = GetDC(window);
	for(u4 x = 0;x <= RES*2;x+=RES){
		for(u4 y = 0;y <= MAP*RES*2;y+=MAP*RES){
			genMap((IVEC2){0,0},x+y,7,-1.0f);
		}
	}
	for(u4 x = player.pos.x - 3.0f;x < player.pos.x + 3.0f;x++){
		for(u4 y = player.pos.y - 3.0f;y < player.pos.y + 3.0f;y++){
			map[x*MAP+y] = 0;
		}
	}
	genTextures();
	CreateThread(0,0,render,0,0,0);
	CreateThread(0,0,physics,0,0,0);
	while(GetMessageA(&Msg,window,0,0)){
		TranslateMessage(&Msg);
		DispatchMessageA(&Msg);
	}
}