#pragma once

#include "small_types.h"

typedef struct{
	f4 x;
	f4 y;
}VEC2;

f4 VEC2length(VEC2 p);
VEC2 VEC2absR(VEC2 p);
VEC2 VEC2divFR(VEC2 p,f4 d);
VEC2 VEC2subVEC2R(VEC2 p,VEC2 p2);
VEC2 VEC2divR(VEC2 p,f4 d);
VEC2 VEC2normalizeR(VEC2 p);
void VEC2addVEC2(VEC2* p,VEC2 p2);
void VEC2subVEC2(VEC2* p,VEC2 p2);
void VEC2div(VEC2* p,f4 d);
void VEC2mul(VEC2* p,f4 m);

VEC2 VEC2divFR(VEC2 p,f4 d);
VEC2 VEC2mulVEC2R(VEC2 p,VEC2 p2);
VEC2 VEC2addVEC2R(VEC2 p,VEC2 p2);
VEC2 VEC2mulR(VEC2 p,f4 m);