#pragma once

#include "small_types.h"
#include "ivec2.h"
#include "vec2.h"
#include "vec3.h"

#pragma comment(lib,"Winmm.lib")

#define PLAYER_SPEED 0.017f

#define WEAPON_LASER_COST 700

#define ENERGY_MAX 6000
#define HEALTH_MAX 100
#define SCRAP_MAX 50

#define ENTITY_REMOVE(TYPE,ID) for(u4 I = ID--;I < TYPE.cnt;I++) TYPE.state[I] = TYPE.state[I+1]; TYPE.cnt--;

#define CHUNK_SIZE 128
#define SIM_SIZE (CHUNK_SIZE*3)
#define CHUNK_SIZE_SURFACE (CHUNK_SIZE*CHUNK_SIZE)
#define SIM_SIZE_SURFACE (SIM_SIZE*SIM_SIZE)

#define PLAYER_MOUSE(X) (VEC2){X.x/8.4375f,CHUNK_SIZE-X.y/8.4375f}
#define PLAYER_SPAWN {CHUNK_SIZE/2+CHUNK_SIZE,CHUNK_SIZE/2+CHUNK_SIZE}

#define LASER_LUMINANCE (VEC3){0.04f,0.005f,0.03f}
#define RD_LASER_LUMINANCE (VEC3){LASER_LUMINANCE.r*8.0f,LASER_LUMINANCE.g*8.0f,LASER_LUMINANCE.b*8.0f}
#define ENT_LASER_LUMINANCE (VEC3){LASER_LUMINANCE.r*0.125f,LASER_LUMINANCE.g*0.125f,LASER_LUMINANCE.b*0.125f}

#define BULLET_LUMINANCE (VEC3){0.1f,0.3f,0.1f}
#define BULLET_SIZE 1.0f

#define PLAYER_LUMINANCE (VEC3){2.0f,0.02f,0.02f}
#define PLAYER_SIZE 2.5f
#define PLAYER_WEAPON_COOLDOWN 60

#define ENEMY_SIZE 1.7f

#define CAM_AREA 0.0f

#define PR_FRICTION 0.9f

#define VK_W 0x57
#define VK_S 0x53
#define VK_A 0x41
#define VK_D 0x44
#define VK_L 0x4C
#define VK_F 0x46

enum{
	ITEM_NOTHING,
	ITEM_MELEE 
};

enum{
	PLAYER_SPRITE,
	BULLET_SPRITE,
	CROSSHAIR_SPRITE,
	ENEMY_SPRITE
};

enum{
	BLOCK_AIR,
	BLOCK_NORMAL,
	BLOCK_LIGHT,
	BLOCK_ENTITY,
	BLOCK_SPRINKLER
};

typedef struct{
	u1 r;
	u1 g;
	u1 b;
}RGB;

typedef struct{
	u1 item_equiped;
	u1 item[9];
}INVENTORY;

typedef struct{
	u4 health;
	u4 energy;
	u4 scrap;
	u4 flashlight;
	u4 weapon_cooldown;
	u4 respawn_countdown;
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
	VEC3 luminance;
	u4 health;
	u1 aggressive;
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
VEC2 getCursorPosGUI();
u1* loadFile(u1* name);
RGB* loadBMP(u1* name);
VEC2 entityPull(VEC2 entity,VEC2 destination,f4 power);
void collision(VEC2* pos,VEC2 vel,f4 size);

extern u1* map;
extern LASERHUB  laser;
extern ENEMYHUB  entity_dark;
extern BLOCKENTITYHUB entity_block;
extern INVENTORY inventory;
extern PLAYER player;
extern CAMERA camera;
extern CAMERA camera_new;
extern RGB*  vram;
extern RGB* texture16;
extern VEC3* vramf;