#pragma once

#include "vec2.h"
#include "vec3.h"

#define GL_DYNAMIC_DRAW 0x88E8

typedef struct{
	VEC2 p1;
	VEC2 tc1;
	VEC2 p2;
	VEC2 tc2;
	VEC2 p3;
	VEC2 tc3;
	VEC2 p4;
	VEC2 tc4;
	VEC2 p5;
	VEC2 tc5;
	VEC2 p6;
	VEC2 tc6;
}QUAD;

void drawSprite(VEC2 pos,VEC2 size,VEC2 texture_pos);
void drawEnemy(VEC2 pos,VEC2 size,VEC2 texture_pos,VEC3 luminance);
void drawMap();
void drawLaser(VEC2 origin,VEC2 destination,VEC3 color);
void drawParticle(VEC2 pos,VEC2 size,VEC3 luminance);
void drawRect(VEC2 pos,VEC2 size,VEC3 color);
void drawString(VEC2 pos,VEC2 size,u1* string);