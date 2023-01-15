#include <math.h>

#include "vec3.h"

VEC3 VEC3mulR(VEC3 p,f4 m){
	return (VEC3){p.x*m,p.y*m,p.z*m};
}

void VEC3mul(VEC3* p,f4 m){
	p->x *= m;
	p->y *= m;
	p->z *= m;
}

VEC3 VEC3addVEC3(VEC3* p,VEC3 p2){
	p->x += p2.x;
	p->y += p2.y;
	p->z += p2.z;
}

f4 VEC3length(VEC3 p){
	return sqrtf(p.x*p.x+p.y*p.y+p.z*p.z);
}

VEC3 VEC3normalizeR(VEC3 p){
	f4 l = VEC3length(p);
	return (VEC3){p.x/l,p.y/l,p.z/l};
}