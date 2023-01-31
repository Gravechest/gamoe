#pragma once

#include "small_types.h"
#include "ivec2.h"
#include "vec2.h"
#include "vec3.h"

#pragma comment(lib,"Winmm.lib")

#define BUILDING_TEXURE_ROWCOUNT 8

#define TILE_TEXTURE_SIZE 4
#define TILE_TEXTURE_SURFACE TILE_TEXTURE_SIZE * TILE_TEXTURE_SIZE

#define ITEM_SPRITE_OFFSET 11

#define WEAPON_LASER_COST 700

#define ENTITY_REMOVE(TYPE,ID) for(u4 I = ID--;I < TYPE.cnt;I++) TYPE.state[I] = TYPE.state[I+1]; TYPE.cnt--;

#define CHUNK_SIZE 128
#define SIM_SIZE (CHUNK_SIZE*3)
#define CHUNK_SIZE_SURFACE (CHUNK_SIZE*CHUNK_SIZE)
#define SIM_SIZE_SURFACE (SIM_SIZE*SIM_SIZE)

#define PLAYER_MOUSE(X) (VEC2){X.x/8.4375f,CHUNK_SIZE-X.y/8.4375f}
#define PLAYER_SPAWN {CHUNK_SIZE/2+CHUNK_SIZE,CHUNK_SIZE/2+CHUNK_SIZE}

#define TORCH_LUMINANCE (VEC3){0.1f,0.05f,0.009f} 

#define LASER_LUMINANCE (VEC3){0.04f,0.005f,0.03f}
#define RD_LASER_LUMINANCE (VEC3){LASER_LUMINANCE.r*8.0f,LASER_LUMINANCE.g*8.0f,LASER_LUMINANCE.b*8.0f}
#define ENT_LASER_LUMINANCE (VEC3){LASER_LUMINANCE.r*0.125f,LASER_LUMINANCE.g*0.125f,LASER_LUMINANCE.b*0.125f}

#define CAM_AREA 0.0f

#define PR_FRICTION 0.9f

#define VK_W 0x57
#define VK_S 0x53
#define VK_A 0x41
#define VK_D 0x44
#define VK_L 0x4C
#define VK_F 0x46
#define VK_E 0x45

enum{
	ITEM_NOTHING,
	ITEM_MELEE,
	ITEM_PICKAXE,
	ITEM_BOMB,
	ITEM_LASER,
	ITEM_STONEDUST,
	ITEM_LOG,
	ITEM_TORCH
};

enum{
	SPRITE_PLAYER,
	SPRITE_BULLET,
	SPRITE_CROSSHAIR,
	SPRITE_ENEMY,
	SPRITE_BLOCK_PARTICLE
};

enum{
	BLOCK_AIR,
	BLOCK_NORMAL,
	BLOCK_LIGHT,
	BLOCK_ENTITY,
	BLOCK_SPRINKLER,
	BLOCK_TREE,
	BLOCK_BUILDING,
	BLOCK_BUILDING_ENTITY
};

enum{
	MENU_GAME,
	MENU_SETTING,
	MENU_CRAFTING_SIMPLE,
	MENU_CONSTRUCT,
	MENU_CRAFTING_BUILDING,
	MENU_DEBUG,
	MENU_CRAFTING_BLOCK
};

typedef struct{
	u1 r;
	u1 g;
	u1 b;
}RGB;

typedef struct{
	VEC2 pos;
	i4 zoom;
	f4 shake;
}CAMERA;

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
	u1 d;
	u1 w;
	u1 a;
	u1 s;
	u1 mouse_right;
}KEYS;

typedef struct{
	u1 health;
	u1 sub_type;
}MAPDATA;

typedef struct{
	u1* type;
	MAPDATA* data;
}MAP;

u4 coordToMap(u4 x,u4 y);
void genMap(IVEC2 crd,u4 offset,u4 depth,f4 value);
VEC2 getCursorPosMap();
VEC2 getCursorPosGUI();
u1* loadFile(u1* name);
RGB* loadBMP(u1* name);
VEC2 entityPull(VEC2 entity,VEC2 destination,f4 power);
void collision(VEC2* pos,VEC2 vel,f4 size);
VEC2 getInventoryPos(u4 place);
u1 pointAABBcollision(VEC2 point,VEC2 aabb,VEC2 size);
u1 AABBcollision(VEC2 pos1,VEC2 pos2,f4 size1,f4 size2);
u1 lineOfSight(VEC2 pos_1,VEC2 pos_2);

extern MAP map;
extern LASERHUB  laser;
extern CAMERA camera;
extern CAMERA camera_new;
extern RGB*  vram;
extern RGB* texture16;
extern RGB* building_texture;
extern VEC3* vramf;
extern KEYS key_pressed;
extern RGB* tile_texture_data;
extern u1 menu_select;