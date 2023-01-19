#include "ray.h"
#include "tmath.h"
#include "math.h"

f4 rayIntersectSquare(VEC2 ray_pos,VEC2 ray_dir,VEC2 square_pos,f4 size){
	VEC2 rel_pos = VEC2subVEC2R(square_pos,ray_pos);
	VEC2 delta = VEC2divFR(rel_pos,1.0f);
	VEC2 n = VEC2mulVEC2R(delta,ray_dir);
	VEC2 k = VEC2mulR(VEC2absR(delta),size);
	VEC2 t1= VEC2subVEC2R(VEC2negR(n),k);
	VEC2 t2= VEC2addVEC2R(VEC2negR(n),k);
	f4 tN = tMaxf(t1.x,t1.y);
	f4 tF = tMinf(t2.x,t2.y);
	if(tN>tF||tF<0.0f) return -1.0f;
	return tN;
}

f4 rayIntersectSphere(VEC2 ray_pos,VEC2 ray_dir,VEC2 sphere_pos,f4 radius){
    VEC2 rel_pos = VEC2subVEC2R(sphere_pos,ray_pos);
	f4 b = VEC2dotR(rel_pos,ray_dir);
	f4 c = VEC2dotR(rel_pos,rel_pos) - radius*radius;
	f4 h = b*b - c;
	if(h<0.0f) return -1.0f;
	h = sqrtf(h);
}

RAY2D ray2dCreate(VEC2 pos,VEC2 dir){
	RAY2D ray;

	ray.pos = pos;
	ray.dir = dir;
	ray.delta = VEC2absR(VEC2divFR(ray.dir,1.0f));

	if(ray.dir.x < 0.0f){
		ray.step.x = -1;
		ray.side.x = (ray.pos.x-(int)ray.pos.x) * ray.delta.x;
	}
	else{
		ray.step.x = 1;
		ray.side.x = ((int)ray.pos.x + 1.0f - ray.pos.x) * ray.delta.x;
	}
	if(ray.dir.y < 0.0f){
		ray.step.y = -1;
		ray.side.y = (ray.pos.y-(int)ray.pos.y) * ray.delta.y;
	}
	else{
		ray.step.y = 1;
		ray.side.y = ((int)ray.pos.y + 1.0f - ray.pos.y) * ray.delta.y;
	}
	ray.square_pos.x = ray.pos.x;
	ray.square_pos.y = ray.pos.y;
	return ray;
}

void ray2dIterate(RAY2D *ray){
	if(ray->side.x < ray->side.y){
		ray->square_pos.x += ray->step.x;
		ray->side.x += ray->delta.x;
		ray->square_side = SQUARE_SIDE_X;
	}
	else{
		ray->square_pos.y += ray->step.y;
		ray->side.y += ray->delta.y;
		ray->square_side = SQUARE_SIDE_Y;
	}
}

VEC2 ray2dGetCoords(RAY2D ray){
	f4 wall;
	switch(ray.square_side){
	case SQUARE_SIDE_X:
		wall = ray.pos.y + (ray.side.x - ray.delta.x) * ray.dir.y;
		if(ray.dir.x > 0.0f) return (VEC2){ray.square_pos.x     ,wall};
		else                 return (VEC2){ray.square_pos.x+1.0f,wall};
	case SQUARE_SIDE_Y:
		wall = ray.pos.x + (ray.side.y - ray.delta.y) * ray.dir.x;
		if(ray.dir.y > 0.0f) return (VEC2){wall,ray.square_pos.y     };
		else                 return (VEC2){wall,ray.square_pos.y+1.0f};
	}
}