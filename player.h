#pragma once

#define ENERGY_MAX 6000
#define HEALTH_MAX 100
#define SCRAP_MAX 50

#define PLAYER_SPEED 0.017f
#define PLAYER_SIZE 2.5f
#define PLAYER_LASER_COOLDOWN 60
#define PLAYER_MELEE_COOLDOWN 30
#define PLAYER_FIST_COOLDOWN 20

#include "vec2.h"
#include "small_types.h"

enum{
	BLOCKHIT_DESTROY,
	BLOCKHIT_NORMAL
};

typedef struct{
	VEC2 vel;
	VEC2 pos;
	i4 health;
	i4 energy;
	i4 scrap;
	u4 flashlight;
	u4 weapon_cooldown;
	u4 respawn_countdown;
	u4 melee_progress;
	u4 fist_side;
}PLAYER;

void playerAttack();
void playerGameTick();
void playerHurt(u4 ammount);
VEC2 playerLookDirection();
VEC2 meleeHitPos();

extern PLAYER player;