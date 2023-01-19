#pragma once

#include "vec2.h"
#include "small_types.h"
#include "ivec2.h"
#include "vec3.h"

#pragma comment(lib,"Winmm.lib")

#define WEAPON_LASER_COST 700

#define ENERGY_MAX 6000

#define PARTICLE_NORMAL        0
#define PARTICLE_ENERGY_INFANT 1
#define PARTICLE_ENERGY_PARENT 2

#define ENTITY_REMOVE(TYPE,ID) for(u4 I = ID--;I < TYPE.cnt;I++) TYPE.state[I] = TYPE.state[I+1]; TYPE.cnt--;

#define CHUNK_SIZE 128
#define SIM_SIZE (CHUNK_SIZE*3)
#define CHUNK_SIZE_SURFACE (CHUNK_SIZE*CHUNK_SIZE)
#define SIM_SIZE_SURFACE (SIM_SIZE*SIM_SIZE)

#define PLAYER_MOUSE(X) (VEC2){X.x/8.4375f,CHUNK_SIZE-X.y/8.4375f}

#define LASER_LUMINANCE (VEC3){0.04f,0.005f,0.03f}
#define RD_LASER_LUMINANCE (VEC3){LASER_LUMINANCE.r*8.0f,LASER_LUMINANCE.g*8.0f,LASER_LUMINANCE.b*8.0f}
#define ENT_LASER_LUMINANCE (VEC3){LASER_LUMINANCE.r*0.125f,LASER_LUMINANCE.g*0.125f,LASER_LUMINANCE.b*0.125f}

#define BULLET_LUMINANCE (VEC3){0.1f,0.3f,0.1f}
#define BULLET_SIZE 1.0f

#define PLAYER_LUMINANCE (VEC3){2.0f,0.02f,0.02f}
#define PLAYER_SIZE 2.5f
#define PLAYER_WEAPON_COOLDOWN 60

#define ENEMY_SPRITE (VEC2){0.5f,0.5f}
#define CROSSHAIR_SPRITE (VEC2){0.0f,0.5f}
#define BULLET_SPRITE (VEC2){0.5f,0.0f}
#define PLAYER_SPRITE (VEC2){0.0f,0.0f}

#define ENEMY_SIZE 1.7f

#define CAM_AREA 0.0f

#define PR_FRICTION 0.9f

#define VK_W 0x57
#define VK_S 0x53
#define VK_A 0x41
#define VK_D 0x44
#define VK_L 0x4C
#define VK_F 0x46

#define WNDOFFX 0
#define WNDOFFY 0

#define WNDX 1080
#define WNDY 1920

typedef struct{
	u1 r;
	u1 g;
	u1 b;
}RGB;

typedef struct{
	u4 flashlight;
	u4 energy;
	u4 weapon_cooldown;
	u4 lightBulletCnt;
	VEC2 vel;
	VEC2 pos;
}PLAYER;

typedef struct{
	VEC2 pos;
	i4 zoom;
}CAMERA;

typedef struct{
	VEC2 vel;
	VEC2 pos;
}BULLET;

typedef struct{
	u4 cnt;
	BULLET* state;
}BULLETHUB;

typedef struct{
	VEC2 vel;
	VEC2 pos;
	VEC3 luminance;
}ENEMY;

typedef struct{
	u4 cnt;
	ENEMY* state;
}ENEMYHUB;

typedef struct{
	VEC2 pos_org;
	VEC2 pos_dst;
	u4 health;
}LASER;

typedef struct{
	u4 cnt;
	LASER* state;
}LASERHUB;

typedef struct{
	VEC3 color;
	VEC2 pos;
	VEC2 vel;
	u4 health;
	u4 type;
	f4 size;
}PARTICLE;

typedef struct{
	u4 cnt;
	PARTICLE* state;
}PARTICLEHUB;

typedef struct{
	u4 countdown;
	IVEC2 pos;
}BLOCKENTITY;

typedef struct{
	u4 cnt;
	BLOCKENTITY* state;
}BLOCKENTITYHUB;

void genMap(IVEC2 crd,u4 offset,u4 depth,f4 value);
VEC2 getCursorPos();
VEC2 getCursorPosMap();

extern u1* map;
extern LASERHUB  laser;
extern PARTICLEHUB particle;
extern BULLETHUB bullet;
extern ENEMYHUB  enemy;
extern PLAYER player;
extern CAMERA camera;
extern CAMERA camera_new;
extern RGB*  vram;
extern RGB* texture16;
extern VEC3* vramf;