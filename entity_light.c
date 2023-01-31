#include <math.h>

#include "entity_light.h"
#include "source.h"
#include "player.h"
#include "ray.h"
#include "tmath.h"
#include "opengl.h"
#include "entity_item.h"
#include "entity_dark.h"

PARTICLEHUB entity_light;

void particleSpawn(VEC3 color,VEC2 position,VEC2 velocity,f4 size,u4 health){
	entity_light.state[entity_light.cnt].color = (VEC3){0.07f,0.02f,0.02f};
	entity_light.state[entity_light.cnt].type = PARTICLE_NORMAL;
	entity_light.state[entity_light.cnt].pos = position;
	entity_light.state[entity_light.cnt].size = size;
	entity_light.state[entity_light.cnt].health = health;
	entity_light.state[entity_light.cnt++].vel = velocity;
}

void spawnLootOrb(u1 type,VEC2 direction,VEC2 position){
	entity_light.state[entity_light.cnt].size = 0.5f;
	entity_light.state[entity_light.cnt].type = LOOT_PARENT;
	entity_light.state[entity_light.cnt].loot_type = type;
	switch(type){
	case LOOT_ENERGY:
		entity_light.state[entity_light.cnt].color = LOOT_ENERGY_COLOR_PARENT;
		break;
	case LOOT_HEALTH:
		entity_light.state[entity_light.cnt].color = LOOT_HEALTH_COLOR_PARENT;
		break;
	case LOOT_SCRAP:
		entity_light.state[entity_light.cnt].color = LOOT_SCRAP_COLOR_PARENT;
		break;
	}
	entity_light.state[entity_light.cnt].vel = VEC2mulR(VEC2rotR(direction,(tRnd()-1.5f)*0.5f),(tRnd()-0.5f)*0.01f);
	entity_light.state[entity_light.cnt].health = 120*60;
	entity_light.state[entity_light.cnt++].pos = position;
}

void entityLightTick(){
	for(u4 i = 0;i < entity_light.cnt;i++){
		switch(entity_light.state[i].type){
		case ENTITY_BOMB:
			if(entity_light.state[i].health--){
				VEC2addVEC2(&entity_light.state[i].pos,entity_light.state[i].vel);
				collision(&entity_light.state[i].pos,entity_light.state[i].vel,entity_light.state[i].size/2.0f);
				VEC2mul(&entity_light.state[i].vel,0.96f);
				if(entity_light.state[i].health<15){
					VEC3mul(&entity_light.state[i].color,0.8f);
					entity_light.state[i].size *= 0.8f;
				}
			}
			//explosion
			else{
				for(f4 j = 0.0f;j < 1.0f;j+=1.0f/255.0f){
					VEC2 direction = (VEC2){cosf(j*PI*2.0f),sinf(j*PI*2.0f)};
					RAY2D ray = ray2dCreate(entity_light.state[i].pos,direction);
					while(ray.square_pos.x >= 0 && ray.square_pos.x < SIM_SIZE && ray.square_pos.y >= 0 && ray.square_pos.y < SIM_SIZE){
						u4 m_pos = coordToMap(ray.square_pos.x,ray.square_pos.y);
						if(map.type[m_pos] == BLOCK_NORMAL){
							blockHit(ray2dGetCoords(ray),m_pos,10);
							break;
						}
						ray2dIterate(&ray);
					}
				}
				gl_queue.message[gl_queue.cnt++].id = GLMESSAGE_WHOLE_MAPEDIT;
				VEC2 spawn_pos = entity_light.state[i].pos;
				for(u4 i = 0;i < 64;i++){
					particleSpawn((VEC3){0.1f,0.01f,0.01f},spawn_pos,VEC2mulR(VEC2rndR(),(tRnd()-1.0f)),0.4f,tRnd()*30.0f);	
				}
				if(lineOfSight(player.pos,entity_light.state[i].pos)){
					f4 dst = VEC2distance(player.pos,entity_light.state[i].pos);
					playerHurt(800.0f/(dst*dst));
				}
				for(u4 j = 0;j < entity_dark.cnt;j++){
					if(lineOfSight(entity_dark.state[j].pos,entity_light.state[i].pos)){
						f4 dst = VEC2distance(entity_dark.state[j].pos,entity_light.state[i].pos);
						entityDarkHurt(j,800.0f/(dst*dst));
					}
				}
				camera_new.shake = 10.0f/VEC2distance(entity_light.state[i].pos,player.pos);
				ENTITY_REMOVE(entity_light,i);
			}
			break;
		case PARTICLE_NORMAL:
			if(entity_light.state[i].health--){
				VEC2addVEC2(&entity_light.state[i].pos,entity_light.state[i].vel);
				collision(&entity_light.state[i].pos,entity_light.state[i].vel,entity_light.state[i].size/2.0f);
				if(entity_light.state[i].health<15){
					VEC3mul(&entity_light.state[i].color,0.8f);
					entity_light.state[i].size *= 0.8;
				}	
			}
			else{
				ENTITY_REMOVE(entity_light,i);
			}
			break;
		case LOOT_INFANT:
			if(entity_light.state[i].health--){
				switch(entity_light.state[i].loot_type){
				case LOOT_ENERGY:
					VEC3addVEC3(&entity_light.state[i].color,LOOT_ENERGY_COLOR_INFANT);
					break;
				case LOOT_HEALTH:
					VEC3addVEC3(&entity_light.state[i].color,LOOT_HEALTH_COLOR_INFANT);
					break;
				}
				entity_light.state[i].size += 0.02f;
			}
			else{
				entity_light.state[i].vel = VEC2rndR();
				entity_light.state[i].type = LOOT_PARENT;
				entity_light.state[i].health = 60*60;
			}
			break;
		case LOOT_PARENT:
			if(entity_light.state[i].health--){
				VEC2addVEC2(&entity_light.state[i].pos,entity_light.state[i].vel);
				collision(&entity_light.state[i].pos,entity_light.state[i].vel,entity_light.state[i].size/2.0f);
				if(VEC2distance(entity_light.state[i].pos,player.pos)<1.0f){
					switch(entity_light.state[i].loot_type){
					case LOOT_SCRAP:
						player.scrap++;
						if(player.energy>SCRAP_MAX) player.energy = SCRAP_MAX;
						break;
					case LOOT_ENERGY:
						player.energy += 200;
						if(player.energy>ENERGY_MAX) player.energy = ENERGY_MAX;
						break;
					case LOOT_HEALTH:
						player.health += 10;
						if(player.health>HEALTH_MAX) player.health = HEALTH_MAX;
						break;
					}
					ENTITY_REMOVE(entity_light,i);
					break;
				}	
				VEC2mul(&entity_light.state[i].vel,PR_FRICTION);
				VEC2addVEC2(&entity_light.state[i].vel,entityPull(entity_light.state[i].pos,player.pos,1.0f));
				if(entity_light.state[i].health<15){
					VEC3mul(&entity_light.state[i].color,0.8f);
					entity_light.state[i].size *= 0.8;
				}	
			}
			else{
				ENTITY_REMOVE(entity_light,i);
			}
			break;
		}
	}
}