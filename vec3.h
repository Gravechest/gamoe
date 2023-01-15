#pragma once

#include "small_types.h"

typedef struct{
	union{
		f4 r;
		f4 x;
	};
	union{
		f4 g;
		f4 y;
	};
	union{
		f4 b;
		f4 z;
	};
}VEC3;

VEC3 VEC3mulR(VEC3 p,f4 m);
void VEC3mul(VEC3* p,f4 m);
VEC3 VEC3addVEC3(VEC3* p,VEC3 p2);
f4 VEC3length(VEC3 p);
VEC3 VEC3normalizeR(VEC3 p);