#include "ray.h"

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
	ray.roundPos.x = ray.pos.x;
	ray.roundPos.y = ray.pos.y;
	return ray;
}

void ray2dIterate(RAY2D *ray){
	if(ray->side.x < ray->side.y){
		ray->roundPos.x += ray->step.x;
		ray->side.x += ray->delta.x;
		ray->hitSide = 0;
	}
	else{
		ray->roundPos.y += ray->step.y;
		ray->side.y += ray->delta.y;
		ray->hitSide = 1;
	}
}

VEC2 ray2dGetCoords(RAY2D ray){
	f4 wall;
	switch(ray.hitSide){
	case 0:
		wall = ray.pos.y + (ray.side.x - ray.delta.x) * ray.dir.y;
		if(ray.dir.x > 0.0f) return (VEC2){ray.roundPos.x     ,wall};
		else                 return (VEC2){ray.roundPos.x+1.0f,wall};
	case 1:
		wall = ray.pos.x + (ray.side.y - ray.delta.y) * ray.dir.x;
		if(ray.dir.y > 0.0f) return (VEC2){wall,ray.roundPos.y     };
		else                 return (VEC2){wall,ray.roundPos.y+1.0f};
	}
}