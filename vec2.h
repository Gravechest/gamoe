#pragma once

#include "small_types.h"

#define VEC2_X 0
#define VEC2_Y 1

typedef struct{
	union{
		struct{
			f4 x;
			f4 y;
		};
		f4 a[2];
	};
}VEC2;

f4 VEC2length(VEC2 p);
f4 VEC2distance(VEC2 p,VEC2 p2);
f4 VEC2dotR(VEC2 p,VEC2 p2);

void VEC2addVEC2(VEC2* p,VEC2 p2);
void VEC2subVEC2(VEC2* p,VEC2 p2);
void VEC2div(VEC2* p,f4 d);
void VEC2mul(VEC2* p,f4 m);
void VEC2sub(VEC2* p,f4 s);
void VEC2add(VEC2* p,f4 a);
void VEC2rot(VEC2* p,f4 rot);

VEC2 VEC2divFR(VEC2 p,f4 d);
VEC2 VEC2mulVEC2R(VEC2 p,VEC2 p2);
VEC2 VEC2addVEC2R(VEC2 p,VEC2 p2);
VEC2 VEC2mulR(VEC2 p,f4 m);
VEC2 VEC2negR(VEC2 p);
VEC2 VEC2rotR(VEC2 p,f4 rot);
VEC2 VEC2subR(VEC2 p,f4 s);
VEC2 VEC2absR(VEC2 p);
VEC2 VEC2divFR(VEC2 p,f4 d);
VEC2 VEC2subVEC2R(VEC2 p,VEC2 p2);
VEC2 VEC2divR(VEC2 p,f4 d);
VEC2 VEC2normalizeR(VEC2 p);
VEC2 VEC2addR(VEC2 p,f4 a);
VEC2 VEC2rndR();