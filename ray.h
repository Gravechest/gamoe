#pragma once

#include "small_types.h"
#include "vec2.h"
#include "ivec2.h"

typedef struct{
	VEC2 pos;
	VEC2 dir;
	VEC2 delta;
	VEC2 side;

	IVEC2 step;
	IVEC2 roundPos;

	i4 hitSide;
}RAY2D;

RAY2D ray2dCreate(VEC2 pos,VEC2 dir);
void ray2dIterate(RAY2D *ray);
VEC2 ray2dGetCoords(RAY2D ray);