#include <math.h>

#include "lighting.h"
#include "ray.h"
#include "source.h"
#include "tmath.h"
#include "entity_dark.h"
#include "entity_item.h"
#include "player.h"
#include "entity_light.h"
#include "inventory.h"
#include "entity_block.h"
#include "construction.h"

u4 entity_shadows_cnt;
IVEC2 entity_shadows[255];

void mapIlluminate(RAY2D ray,VEC3 color){
	while(ray.square_pos.x >= 0 && ray.square_pos.x < camera.zoom && ray.square_pos.y >= 0 && ray.square_pos.y < camera.zoom){
		switch(map.type[coordToMap(ray.square_pos.x+camera.pos.x,ray.square_pos.y+camera.pos.y)]){
		case BLOCK_ENTITY:
			VEC3addVEC3(&vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x],color);
			for(u4 i = 0;i < entity_item.cnt;i++){
				f4 hit_area = (ENTITY_ITEM_SIZE/2.0f+1.0f);
				VEC2 entity_dark_pos_rel = VEC2subVEC2R(VEC2subVEC2R(entity_item.state[i].pos,camera.pos),(VEC2){ray.square_pos.x,ray.square_pos.y});
				if(entity_dark_pos_rel.x - hit_area < 0.0f && entity_dark_pos_rel.x + hit_area > 0.0f &&
				entity_dark_pos_rel.y - hit_area < 0.0f && entity_dark_pos_rel.y + hit_area > 0.0f){
					if(rayIntersectSquare(entity_item.state[i].pos,ray.dir,VEC2addVEC2R(ray.pos,camera.pos),ENEMY_SIZE*0.5) != -1.0f){
						VEC3addVEC3(&entity_item.state[i].luminance,color);
					}
				}
			}
			for(u4 i = 0;i < entity_dark.cnt;i++){
				f4 hit_area = (ENEMY_SIZE/2.0f+1.0f);
				VEC2 entity_dark_pos_rel = VEC2subVEC2R(VEC2subVEC2R(entity_dark.state[i].pos,camera.pos),(VEC2){ray.square_pos.x,ray.square_pos.y});
				if(entity_dark_pos_rel.x - hit_area < 0.0f && entity_dark_pos_rel.x + hit_area > 0.0f &&
				entity_dark_pos_rel.y - hit_area < 0.0f && entity_dark_pos_rel.y + hit_area > 0.0f){
					if(rayIntersectSquare(entity_dark.state[i].pos,ray.dir,VEC2addVEC2R(ray.pos,camera.pos),ENEMY_SIZE*0.5) != -1.0f){
						VEC3addVEC3(&entity_dark.state[i].luminance,color);
						return;
					}
				}
			}
			break;
		case BLOCK_AIR:
			VEC3addVEC3(&vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x],color);
			break;
		case BLOCK_SPRINKLER:
			VEC3addVEC3(&vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x],VEC3mulR(color,0.25f));
			break;
		case BLOCK_BUILDING:
		case BLOCK_NORMAL:
		case BLOCK_TREE:
			VEC3addVEC3(&vramf[ray.square_pos.y*camera.zoom+ray.square_pos.x],VEC3mulR(color,4.0f));
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
			if(block.x>SIM_SIZE||block.y>SIM_SIZE||block.x<0||block.y<0||map.type[coordToMap(block.x,block.y)]==BLOCK_NORMAL) break;
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

void lightsourceBlockEmit(VEC3 color,VEC2 pos,u4 ammount){
	VEC2subVEC2(&pos,(VEC2){(u4)camera.pos.x,(u4)camera.pos.y});
	f4 offset = tRnd();
	if(pos.x < 1.0f || pos.y < 1.0f || pos.x > camera.zoom || pos.y > camera.zoom){
		for(f4 i = offset;i < 1.0f+offset;i+=1.0f/ammount){
			VEC2 direction = (VEC2){cosf(i*PI*2.0f),sinf(i*PI*2.0f)};
			if(rayIntersectSquare(pos,direction,(VEC2){camera.zoom/2.0f,camera.zoom/2.0f},(f4)camera.zoom/2.0f) != -1.0f){
				RAY2D ray = ray2dCreate(pos,direction);
				ray2dIterate(&ray);
				while((ray.square_pos.x < 0 || ray.square_pos.x >= camera.zoom || ray.square_pos.y < 0 || ray.square_pos.y >= camera.zoom)){
					IVEC2 block = {(ray.square_pos.x+(u4)camera.pos.x),(ray.square_pos.y+(u4)camera.pos.y)};
					if(block.x>SIM_SIZE||block.y>SIM_SIZE||block.x<0||block.y<0||map.type[coordToMap(block.x,block.y)]==BLOCK_NORMAL) break;
					ray2dIterate(&ray);
				}
				mapIlluminate(ray,color);
			}
			illuminateOutside(color,pos,i);
		}
	}
	else{
		for(f4 i = offset;i < 1.0f+offset;i+=1.0f/ammount){
			RAY2D ray = ray2dCreate(pos,(VEC2){cosf(i*PI*2.0f),sinf(i*PI*2.0f)});
			ray2dIterate(&ray);
			mapIlluminate(ray,color);
		}	
	}
}

void lighting(){
	entity_shadows_cnt = 0;
	for(u4 i = 0;i < entity_item.cnt;i++){
		f4 dv = ENTITY_ITEM_SIZE/(u4)(ENTITY_ITEM_SIZE+1);
		for(f4 x = entity_item.state[i].pos.x-ENTITY_ITEM_SIZE*0.5f;x <= entity_item.state[i].pos.x+ENTITY_ITEM_SIZE*0.5f;x+=dv){
			for(f4 y = entity_item.state[i].pos.y-ENTITY_ITEM_SIZE*0.5f;y <= entity_item.state[i].pos.y+ENTITY_ITEM_SIZE*0.5f;y+=dv){
				if(map.type[coordToMap(x,y)]==BLOCK_AIR){
					map.type[coordToMap(x,y)] = BLOCK_ENTITY;
					entity_shadows[entity_shadows_cnt++] = (IVEC2){x,y};
				}
			}
		}
	}
	for(u4 i = 0;i < entity_dark.cnt;i++){
		f4 dv = ENEMY_SIZE/(u4)(ENEMY_SIZE+1);
		for(f4 x = entity_dark.state[i].pos.x-ENEMY_SIZE*0.5f;x <= entity_dark.state[i].pos.x+ENEMY_SIZE*0.5f;x+=dv){
			for(f4 y = entity_dark.state[i].pos.y-ENEMY_SIZE*0.5f;y <= entity_dark.state[i].pos.y+ENEMY_SIZE*0.5f;y+=dv){
				if(map.type[coordToMap(x,y)]==BLOCK_AIR){
					map.type[coordToMap(x,y)] = BLOCK_ENTITY;
					entity_shadows[entity_shadows_cnt++] = (IVEC2){x,y};
				}
			}
		}
	}
	for(u4 i = 0;i < entity_block.cnt;i++){
		u4 m_loc = coordToMap(entity_block.state[i].pos.x,entity_block.state[i].pos.y);
		if(map.data[m_loc].sub_type == CONSTRUCTION_LAMP){
			VEC2 l_pos = {entity_block.state[i].pos.x+0.5f,entity_block.state[i].pos.y+0.5f};
			lightsourceBlockEmit((VEC3){1.0f,1.0f,1.0f},l_pos,1024);
		}
	}
	for(u4 i = 0;i < entity_light.cnt;i++){
		lightsourceEmit(entity_light.state[i].color,entity_light.state[i].pos,1024);
	}
	for(u4 i = 0;i < laser.cnt;i++){
		VEC2 normalize_pos = VEC2subVEC2R(laser.state[i].pos_dst,laser.state[i].pos_org);
		VEC2 direction = VEC2normalizeR(normalize_pos);
		u4 dst = VEC2length(normalize_pos);
		VEC2 pos = laser.state[i].pos_org;
		for(u4 j = 0;j < dst;j++){
			lightsourceEmit(LASER_LUMINANCE,pos,1024);
			VEC2addVEC2(&pos,direction);
		}
		if(laser.state[i].health == 1) laser.state[i].health = 0;
	}
	static f4 flickering = 0.0f;
	switch(inventory.item_secundary.type){
	case ITEM_TORCH:
		lightsourceEmit(VEC3mulR(TORCH_LUMINANCE,flickering),player.pos,1024);
		flickering += tRnd() + 4.0f;
		flickering /= 4.0f;
		break;
	}
	for(u4 i = 0;i < entity_shadows_cnt;i++){
		map.type[coordToMap(entity_shadows[i].x,entity_shadows[i].y)] = BLOCK_AIR;
	}
	if(player.melee_progress){
		VEC2 end_pos = meleeHitPos();
		VEC2subVEC2(&end_pos,camera.pos);
		VEC2sub(&end_pos,0.5f);
		VEC3mul(&vramf[(u4)(end_pos.y+0)*camera.zoom+(u4)(end_pos.x+0)],0.5f);
		VEC3mul(&vramf[(u4)(end_pos.y+1)*camera.zoom+(u4)(end_pos.x+0)],0.5f);
		VEC3mul(&vramf[(u4)(end_pos.y+0)*camera.zoom+(u4)(end_pos.x+1)],0.5f);
		VEC3mul(&vramf[(u4)(end_pos.y+1)*camera.zoom+(u4)(end_pos.x+1)],0.5f);
	}
}