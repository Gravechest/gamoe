#include <intrin.h>

#include "tmath.h"

i4 tHash(i4 x){
	x += (x << 10);
	x ^= (x >> 6);
	x += (x << 3);
	x ^= (x >> 11);
	x += (x << 15);
	return x;
}

f4 tRnd(){
	union p {
		f4 f;
		i4 u;
	}r;
	r.u = tHash(__rdtsc());
	r.u &= 0x007fffff;
	r.u |= 0x3f800000;
	return r.f;
}

i4 tRndi(){
	return tHash(__rdtsc());
}

f4 tFloorf(f4 p){
	i4 n = (i4)p;
    f4 d = (f4)n;
    if (d == p || p >= 0) return d;
    else                  return d - 1;
}

f4 tFract(f4 p){
	return p - tFloorf(p);
}

i4 tMax(i4 p,i4 p2){
	return p > p2 ? p : p2;
}

i4 tMin(i4 p,i4 p2){
	return p < p2 ? p : p2;
}

f4 tMaxf(f4 p,f4 p2){
	return p > p2 ? p : p2;
}

f4 tMinf(f4 p,f4 p2){
	return p < p2 ? p : p2;
}

f4 tAbsf(f4 p){
	return p < 0.0f ? -p : p;
}

f4 tInvf(f4 p){
	return 1.0f/p;
}