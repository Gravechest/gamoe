#pragma once

#include "small_types.h"
#include "vec2.h"
#include "ivec2.h"

#define SQUARE_SIDE_X 0
#define SQUARE_SIDE_Y 1

typedef struct{
	VEC2 pos;
	VEC2 dir;
	VEC2 delta;
	VEC2 side;

	IVEC2 step;
	IVEC2 square_pos;

	i4 square_side;
}RAY2D;

RAY2D ray2dCreate(VEC2 pos,VEC2 dir);
void ray2dIterate(RAY2D *ray);
VEC2 ray2dGetCoords(RAY2D ray);

f4 rayIntersectSquare(VEC2 ray_pos,VEC2 ray_dir,VEC2 square_pos,f4 size);
f4 rayIntersectSphere(VEC2 ray_pos,VEC2 ray_dir,VEC2 sphere_pos,f4 radius);