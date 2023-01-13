#include "vec2.h"
#include "math.h"

f4 VEC2length(VEC2 p){
	return sqrtf(p.x*p.x+p.y*p.y);
}

VEC2 VEC2negR(VEC2 p){
	return (VEC2){-p.x,-p.y};
}

VEC2 VEC2absR(VEC2 p){
	p.x = p.x < 0.0f ? -p.x : p.x;
	p.y = p.y < 0.0f ? -p.y : p.y;
	return p;
}

VEC2 VEC2divFR(VEC2 p,f4 d){
	return (VEC2){d/p.x,d/p.y};
}

VEC2 VEC2addVEC2R(VEC2 p,VEC2 p2){
	return (VEC2){p.x+p2.x,p.y+p2.y};
}

VEC2 VEC2subVEC2R(VEC2 p,VEC2 p2){
	return (VEC2){p.x-p2.x,p.y-p2.y};
}

VEC2 VEC2mulVEC2R(VEC2 p,VEC2 p2){
	return (VEC2){p.x*p2.x,p.y*p2.y};
}

VEC2 VEC2divR(VEC2 p,f4 d){
	return (VEC2){p.x/d,p.y/d};
}

VEC2 VEC2mulR(VEC2 p,f4 m){
	return (VEC2){p.x*m,p.y*m};
}

VEC2 VEC2normalizeR(VEC2 p){
	f4 l = VEC2length(p);
	return (VEC2){p.x/l,p.y/l};
}

void VEC2addVEC2(VEC2* p,VEC2 p2){
	p->x += p2.x;
	p->y += p2.y;
}

void VEC2subVEC2(VEC2* p,VEC2 p2){
	p->x -= p2.x;
	p->y -= p2.y;
}

void VEC2add(VEC2* p,f4 a){
	p->x += a;
	p->y += a;
}

void VEC2sub(VEC2* p,f4 s){
	p->x -= s;
	p->y -= s;
}

void VEC2div(VEC2* p,f4 d){
	p->x /= d;
	p->y /= d;
}

void VEC2mul(VEC2* p,f4 m){
	p->x *= m;
	p->y *= m;
}