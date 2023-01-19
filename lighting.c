#include <math.h>

#include "lighting.h"
#include "ray.h"
#include "source.h"
#include "tmath.h"

void mapIlluminate(RAY2D ray,VEC3 color){
	while(ray.square_pos.x >= 0 && ray.square_pos.x < camera.zoom && ray.square_pos.y >= 0 && ray.square_pos.y < camera.zoom){
		switch(map[(ray.square_pos.y+(u4)camera.pos.y)*SIM_SIZE+(ray.square_pos.x+(u4)camera.pos.x)]){
		case BLOCK_ENTITY:
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].r+=color.r;
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].g+=color.g;
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].b+=color.b;
			for(u4 i = 0;i < enemy.cnt;i++){
				f4 hit_area = (ENEMY_SIZE/2.0f+1.0f);
				VEC2 enemy_pos_rel = VEC2subVEC2R(VEC2subVEC2R(enemy.state[i].pos,camera.pos),(VEC2){ray.square_pos.x,ray.square_pos.y});
				if(enemy_pos_rel.x - hit_area < 0.0f && enemy_pos_rel.x + hit_area > 0.0f &&
				enemy_pos_rel.y - hit_area < 0.0f && enemy_pos_rel.y + hit_area > 0.0f){
					if(rayIntersectSquare(VEC2addVEC2R(ray.pos,camera.pos),ray.dir,enemy.state[i].pos,ENEMY_SIZE*0.5) != -1.0f){
						VEC3addVEC3(&enemy.state[i].luminance,color);
						return;
					}
				}
			}
			break;
		case BLOCK_AIR:
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].r+=color.r;
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].g+=color.g;
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].b+=color.b;
			break;
		case BLOCK_SPRINKLER:
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].r+=color.r*0.25f;
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].g+=color.g*0.25f;
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].b+=color.b*0.25f;
			break;
		case BLOCK_NORMAL:
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].r+=color.r*4.0f;
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].g+=color.g*4.0f;
			vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x].b+=color.b*4.0f;
			return;
		}
		ray2dIterate(&ray);
	}
}

void illuminateOutside(VEC3 color,VEC2 pos,f4 angle){
	VEC2 direction = (VEC2){cosf(angle*PI*2.0f),sinf(angle*PI*2.0f)};
	if(rayIntersectSquare(pos,direction,(VEC2){camera.zoom/2.0f,camera.zoom/2.0f},(f4)camera.zoom/2.0f) != -1.0f){
		RAY2D ray = ray2dCreate(pos,direction);
		while((ray.square_pos.x < 0 || ray.square_pos.x >= camera.zoom || ray.square_pos.y < 0 || ray.square_pos.y >= camera.zoom)){
			IVEC2 block = {(ray.square_pos.x+(u4)camera.pos.x),(ray.square_pos.y+(u4)camera.pos.y)};
			if(block.x>SIM_SIZE||block.y>SIM_SIZE||block.x<0||block.y<0||map[block.y*SIM_SIZE+block.x]==BLOCK_NORMAL) break;
			ray2dIterate(&ray);
		}
		mapIlluminate(ray,color);
	}
}

void illuminateInside(VEC3 color,VEC2 pos,f4 angle){
	RAY2D ray = ray2dCreate(pos,(VEC2){cosf(angle*PI*2.0f),sinf(angle*PI*2.0f)});
	mapIlluminate(ray,color);
}

void lightsourcePartEmit(VEC3 color,VEC2 pos,VEC2 angle,u4 ammount,f4 wideness){
	f4 tan_angle = atan2f(angle.y,angle.x) / (PI*2.0f);
	tan_angle -= wideness/2.0f;
	VEC2subVEC2(&pos,(VEC2){(u4)camera.pos.x,(u4)camera.pos.y});
	if(pos.x < 1.0f || pos.y < 1.0f || pos.x > camera.zoom || pos.y > camera.zoom){
		for(f4 i = tan_angle;i < wideness+tan_angle;i+=wideness/ammount){
			illuminateOutside(color,pos,i);
		}
	}
	else{
		for(f4 i = tan_angle;i < wideness+tan_angle;i+=wideness/ammount){
			illuminateInside(color,pos,i);
		}	
	}
}

void lightsourceEmit(VEC3 color,VEC2 pos,u4 ammount){
	VEC2subVEC2(&pos,(VEC2){(u4)camera.pos.x,(u4)camera.pos.y});
	f4 offset = tRnd();
	if(pos.x < 1.0f || pos.y < 1.0f || pos.x > camera.zoom || pos.y > camera.zoom){
		for(f4 i = offset;i < 1.0f+offset;i+=1.0f/ammount){
			illuminateOutside(color,pos,i);
		}
	}
	else{
		for(f4 i = offset;i < 1.0f+offset;i+=1.0f/ammount){
			illuminateInside(color,pos,i);
		}	
	}
}